var fs = require('fs');
var path = require('path');
var os = require('os');
var connect = require('net').connect;

//
// cleanup all child processes on exit (but not if parent process is killed by signal 9 - no way of handling that)
//

var processes = {};
var exit_cb = function () {
    for (var proc_pid in processes) {
        var proc = processes[proc_pid];
        if (proc) {
            proc.kill('SIGTERM');
        }
    }
    processes = {};
};

process.on('exit', exit_cb);
process.on('uncaughtException', exit_cb);
process.on('SIGINT', exit_cb);
process.on('SIGTERM', exit_cb);
process.on('SIGUSR1', exit_cb);
process.on('SIGUSR2', exit_cb);

//
// simple Node.js-based origin server
//

origin_new = function (name, config, log_cb) {
    if (!config) {
        throw "Missing config";
    }

    //
    // Private variables, functions, and setup
    //

    if (!log_cb) {
        log_cb = function (thresh, data) {
        };
    }

    var server = require('http').createServer(function (request, response) {
        log_cb("DEBUG", "Origin received request: " + request.method + " " + request.url + ", headers: " + request.rawHeaders);

        // Send 404 if the method or path doesn't map to an action

        if (!config.actions || !config.actions[request.method] ||
            !config.actions[request.method][request.url]) {
            log_cb('DEBUG', '404: ' + request.method + ' ' + request.url);
            response.writeHead(404, {});
            response.end();
            return;
        }

        var action = config.actions[request.method][request.url];

        // Recycle the response chunk for the action

        if (!action.response_chunk) {
            var chunk_size_bytes = action.chunk_size_bytes || 1024;
            var chunk_byte_value = action.chunk_byte_value || 42;

            action.response_chunk = new Buffer(chunk_size_bytes);
            action.response_chunk.fill(chunk_byte_value);
        }

        // Send the configured response

        // Config settings. Default as necessary
        var delay_first_chunk_millis = action.delay_first_chunk_millis || 0;
        var num_chunks = action.num_chunks || 0;
        var delay_between_chunk_millis = action.delay_between_chunk_millis || 0;
        var status_code = action.status_code || 200;
        var headers = action.headers || {};

        // Per-action state.
        var response_chunk = action.response_chunk;

        // Per-transaction state.
        var chunks_sent = 0;

        // Send headers

        response.writeHead(status_code, headers);

        // Send no chunks

        if (chunks_sent >= num_chunks) {
            response.end();
            return;
        }

        // Send n chunks

        var send_chunk = function () {
            ++chunks_sent;

            // Last chunk

            if (chunks_sent >= num_chunks) {
                response.end(response_chunk);
                return;
            }

            // Intermediate chunk

            response.write(response_chunk);

            if (0 < delay_between_chunk_millis) {
                setTimeout(send_chunk, delay_between_chunk_millis);
            } else {
                process.nextTick(send_chunk);
            }
        };

        if (0 < delay_first_chunk_millis) {
            setTimeout(send_chunk, delay_first_chunk_millis);
        } else {
            process.nextTick(send_chunk);
        }
    });

    // Listen on all endpoints

    // TODO HTTPS

    if (!config.endpoints || !config.endpoints.http || !config.endpoints.http.port) {
        log_cb("ERROR", "Config is missing http port for origin");
        throw "Config is missing http port for origin";
    }

    server.listen(config.endpoints.http.port);

    log_cb('INFO', "origin is listening on port " + config.endpoints.http.port);

    //
    // Public variables and functions
    //

    return {
        stop: function () {
            server.close();
            log_cb('INFO', "origin '" + name + "' has been stopped");
        },
        name: name,
        config: config
    }
}

//
// test framework entry point
//

module.exports.tf_new = function (args) {
    if (!args) {
        throw "Missing args";
    }

    if (!args.config) {
        throw "config is mandatory";
    }

    //
    // Private variables and functions
    //

    var log_cb = args.log_cb;

    if (!log_cb) {
        log_cb = function (thresh, data) {
        };
    }

    var config = args.config;
    var origins = {} // name to origin objects
    var startup_hostports = []; // array of { port: 123, host: 'foo.com' } pairs
    var shutdown_hostports = []; // array of { port: 123, host: 'foo.com' } pairs

    var rm_rf = function (base_path) {
        if (!fs.existsSync(base_path)) {
            return;
        }

        var files = fs.readdirSync(base_path);

        for (var i = 0; i < files.length; ++i) {
            var path = base_path + "/" + files[i];

            if (fs.lstatSync(path).isDirectory()) {
                rm_rf(path);
                continue;
            }

            fs.unlinkSync(path);
        }

        fs.rmdirSync(base_path);
    };

    var checkports = function (hostports, connect_is_success, done) {
        var error_cb = function () {
            if (!hostports[0]) {
                return;
            }

            var hostport = hostports[0];

            // for checking process startup (wait until connect succeeds)

            if (connect_is_success) {
                log_cb('DEBUG', 'Waiting for ' + hostport.host + ':' + hostport.port + " to open");

                hostport['retries'] = hostport['retries'] - 1;
                if (0 >= hostport['retries']) {
                    throw "Cannot connect to: " + hostport.host + ':' + hostport.port;
                }

                setTimeout(function () {
                    var client = connect(hostport.port, hostport.host, connected_cb);
                    client.on('error', error_cb);
                }, 100);

                return;
            }

            // for checking process shutdown (retry until connect fails)

            log_cb('DEBUG', hostport.host + ':' + hostport.port + " is closed");

            hostports.shift();
            if (0 >= hostports.length) {
                done();
                return;
            }

            checkports(hostports, connect_is_success, done);
        };

        var connected_cb = function () {
            var hostport = hostports[0];

            // for checking process startup (wait until connect succeeds)

            if (connect_is_success) {
                log_cb('DEBUG', hostport.host + ':' + hostport.port + " is open");

                hostports.shift();

                if (0 >= hostports.length) {
                    done();
                    return;
                }

                checkports(hostports, connect_is_success, done);
                return;
            }

            // for checking process shutdown (retry until connect fails)
            log_cb('DEBUG', 'Waiting for ' + hostport.host + ':' + hostport.port + " to close");

            hostport['retries'] = hostport['retries'] - 1;
            if (0 >= hostport['retries']) {
                throw "Connected to: " + hostport.host + ':' + hostport.port;
            }

            setTimeout(function () {
                var client = connect(hostport.port, hostport.host, connected_cb);
                client.on('error', error_cb);
            }, 100);
        };

        if (0 >= hostports.length) {
            done();
            return;
        }

        var hostport = hostports[0];

        if (connect_is_success) {
            log_cb('DEBUG', 'Waiting for ' + hostport.host + ':' + hostport.port + " to open");
        } else {
            log_cb('DEBUG', 'Waiting for ' + hostport.host + ':' + hostport.port + " to close");
        }

        var client = connect(hostport, connected_cb);
        client.on('error', error_cb);
    };

    var chomp = function (buf) {
        var i = buf.length - 1;

        for (; i >= 0; --i) {
            if ('\n' !== buf[i] && '\r' !== buf[i]) {
                break;
            }
        }

        return buf.slice(0, i);
    };

    //
    // Public variables and functions
    //

    return {
        start: function (done) {
            var child_process = require('child_process');
            var tmp_file = null;

            for (var proc_name in config.servers) {
                var proc_conf = config.servers[proc_name];
                var proc_type = proc_conf.type;
                var proc;

                if (false === proc_conf.spawn) {
                    continue;
                }

                switch (proc_type) {
                    case 'async process':
                        var args = [];
                        for (key in proc_conf.args) {
                            args.push(key);
                            var value = proc_conf.args[key];
                            if (null != value) {
                                args.push(value);
                            }
                        }

                        Array.from(proc_conf.args, ([key, value]) => (key, value))

                        var env = new Map();
                        for (key in proc_conf.env) {
                            env[key] = proc_conf.env[key];
                        }

                        log_cb('INFO', "Process arguments: " + JSON.stringify(args));
                        log_cb('INFO', "Process environment: " + JSON.stringify(env));

                        proc = child_process.spawn(proc_conf.path, args, {
                            env: env,
                            maxBuffer: 42 * 1024 * 1024,
                            stdio: ['ignore', 'inherit', 'inherit']
                        });

                        (function () {
                            var name = proc_name;
                            var pid = proc.pid;

                            proc.on('exit', function (code, signal) {
                                delete processes[pid];
                                log_cb(code === 0 ? 'INFO' : 'ERROR', 'Process \'' + name + '\' exited with ' + (code || signal) + ', pid=' + pid);
                            });
                        })();

                        processes[proc.pid] = proc;

                        break;

                    case 'sync process':
                        var args = [];
                        for (key in proc_conf.args) {
                            args.push(key);
                            var value = proc_conf.args[key];
                            if (null != value) {
                                args.push(value);
                            }
                        }

                        Array.from(proc_conf.args, ([key, value]) => (key, value))

                        var env = new Map();
                        for (key in proc_conf.env) {
                            env[key] = proc_conf.env[key];
                        }

                        log_cb('INFO', "Process arguments: " + JSON.stringify(args));
                        log_cb('INFO', "Process environment: " + JSON.stringify(env));

                        result = child_process.spawnSync(proc_conf.path, args, {
                            env: env,
                            maxBuffer: 42 * 1024 * 1024,
                            stdio: ['ignore', 'inherit', 'inherit']
                        });

                        log_cb('INFO', "Sync process finished: " + result.toString())

                        if (done) {
                            done(result);
                        }

                        return result;

                    case 'node origin':

                        origins[proc_name] = origin_new(proc_name, proc_conf, log_cb);
                        break;

                    default:
                        throw "Unknown process type: " + proc_type;
                }

                // used to detect when the server is up.
                for (var endpoint_name in proc_conf.endpoints) {
                    var endpoint = proc_conf.endpoints[endpoint_name];
                    startup_hostports.push({
                        port: endpoint.port,
                        host: endpoint.hostname,
                        retries: 10
                    });
                    shutdown_hostports.push({
                        port: endpoint.port,
                        host: endpoint.hostname,
                        retries: 10
                    });
                }
            }

            checkports(startup_hostports, true, done);
        },
        stop: function (done) {
            exit_cb();

            for (var origin_name in origins) {
                log_cb('DEBUG', "Stopping origin: " + origin_name);
                var origin = origins[origin_name];
                origin.stop();
            }
            origins = {}

            checkports(shutdown_hostports, false, done);
        },
        config: config
    };
};

