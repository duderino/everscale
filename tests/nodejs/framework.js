// TODO only log on log_thresh.   add guard statements

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

    var fs = require('fs');
    var config = args.config;
    var servers = {}; // pid to proc object

    var exit_cb = function (exit_code) {
        for (var proc_name in servers) {
            var proc = servers[proc_name];
            if (!proc) {
                continue;
            }

            log_cb('DEBUG', "Stopping process: pid=" + proc.pid);
            proc.kill('SIGTERM');
        }

        servers = {};

        log_cb('INFO', 'Exiting: code=' + exit_code);
    };

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

    var checkports = function (done, hostports) {
        var connect = require('net').connect;

        var connected_cb = function () {
            var hostport = hostports[0];

            log_cb('DEBUG', 'Connected to: ' + hostport.host + ':' + hostport.port);

            hostports.shift();

            if (0 >= hostports.length) {
                done();
                return;
            }

            checkports(done, hostports);
        };

        var error_cb = function () {
            if (!hostports[0]) {
                return;
            }

            var hostport = hostports[0];
            hostport['retries'] = hostport['retries'] - 1;

            if (0 == hostport['retries'] % 27) {
                log_cb('DEBUG', 'Cannot connect to: ' + hostport.host + ':' + hostport.port);
            }

            if (0 >= hostport['retries']) {
                throw "Cannot connect to: " + hostport.host + ':' + hostport.port;
            }

            var client = connect(hostport, connected_cb);

            client.on('error', error_cb);
        };

        if (0 >= hostports.length) {
            done();
            return;
        }

        var hostport = hostports[0];

        log_cb('DEBUG', 'Checking: ' + hostport.host + ':' + hostport.port);

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
            process.on('exit', exit_cb);

            process.on('uncaughtException', function (error) {
                log_cb('ERROR', 'Caught exception: ' +
                    error.stack || error.toString());
                exit_cb(1);
                process.exit(1);
            });

            process.on('SIGTERM', function () {
                process.exit(0);
            });

            process.on('SIGINT', function () {
                process.exit(0);
            });

            var child_process = require('child_process');
            var hostports = []; // array of { port: 123, host: 'foo.com' } pairs

            for (var proc_name in config.servers) {
                var proc_conf = config.servers[proc_name];
                var proc_type = proc_conf.type;
                var proc;

                if (false === proc_conf.spawn) {
                    continue;
                }

                switch (proc_type) {
                    case 'executable':
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

                        proc = child_process.spawn(proc_conf.path, args, {env: env, maxBuffer: 42 * 1024 * 1024});
                        break;

                    case 'node':

                        var script = proc_conf.script;

                        if (!script) {
                            throw "Node process " + proc_name + " missing script";
                        }

                        var node_args = [script].concat(proc_conf.args);

                        log_cb('INFO', 'Spawn ' + proc_name + ': ' + 'node ' + node_args.join(' '));

                        proc = child_process.spawn('node', node_args, {env: process.env});

                        break;

                    case 'origin':

                        if (!args.origin_path) {
                            throw "Path to origin.js is mandatory";
                        }

                        var origin_args = [
                            args.origin_path,
                            proc_name,
                            args.config_path
                        ];

                        log_cb('INFO', 'Spawn ' + proc_name + ': ' + 'node ' + origin_args.join(' '));

                        proc = child_process.spawn('node', origin_args, {});

                        break;

                    case 'other':

                        var exe = proc_conf.executable;

                        if (!exe) {
                            throw "Other process " + proc_name + " missing executable";
                        }

                        var other_args = proc_conf.args || [];

                        log_cb('INFO', 'Spawn ' + proc_name + ': ' + exe + ' ' + other_args.join(' '));

                        proc = child_process.spawn(exe, other_args, {env: process.env});

                        break;

                    default:
                        throw "Unknown process type: " + proc_type;
                }

                // used to detect when the server is up.
                for (var endpoint_name in proc_conf.endpoints) {
                    var endpoint = proc_conf.endpoints[endpoint_name];

                    hostports.push({
                        port: endpoint.port,
                        host: endpoint.hostname,
                        retries: 1000
                    });
                }

                if (!proc) {
                    continue;
                }

                (function () {
                    var name = proc_name;
                    var pid = proc.pid;

                    proc.stdout.on('data', function (data) {
                        log_cb('DEBUG', name + ': ' + chomp(data));
                    });

                    proc.stderr.on('data', function (data) {
                        log_cb('WARN', name + ': ' + chomp(data));
                    });

                    proc.on('close', function (code) {
                        delete servers[pid];

                        if (code === 0) {
                            log_cb('INFO', name + ' exited: code=' + code + ', pid=' + pid);
                            return;
                        }

                        log_cb('ERROR', name + ' exited: code=' + code + ', pid=' + pid);
                        process.exit(1);
                    });
                })();

                servers[proc.pid] = proc;
            }

            if (done) {
                checkports(done, hostports);
            }
        },
        stop: function (done) {
            if (done) {
                done();
            }
        },
        config: config
    };
};

