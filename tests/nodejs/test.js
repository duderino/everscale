var assert = require("assert");
var http = require('http');
const tls = require('tls');
var https = require('https');
var framework = require('./framework.js');
var fs = require('fs');

var log = function (thresh, data) {
    console.error(thresh + ': ' + data);
};

var root_dir = "../..";
var ca_path = root_dir + "/base/tests/ca.crt";
var ca_bytes = fs.readFileSync(ca_path);

var connections = 50;
var requests_per_connection = 50;

var httpAgent = new http.Agent({ keepAlive: true })
var httpsAgent = new https.Agent({ keepAlive: true })

describe('Interoperability tests with Node.js', function () {
    it('Node.js client to Everscale proxy to Everscale origin (HTTP 1.1)', function (done) {
        var origin_port = 8080;
        var proxy_port = 8081;
        var expected_status_code = 200;
        var expected_response_size = 1024;

        var tf = framework.tf_new({
            log_cb: log,
            config: {
                "servers": {
                    "Everscale proxy": {
                        "type": "async process",
                        "path": root_dir + "/data-plane/proxy/http-proxy",
                        "args": {
                            "--proxyThreads": 1,
                            "--proxyPort": proxy_port,
                            "--originPort": origin_port,
                            "--proxyTimeoutMsec": 30000,
                            "--secure": 0,
                            "--logWarn": null
                        },
                        "endpoints": {
                            "http": {
                                "type": "http",
                                "hostname": "localhost",
                                "port": proxy_port
                            }
                        }
                    },
                    "Everscale origin": {
                        "type": "async process",
                        "path": root_dir + "/data-plane/origin/http-origin",
                        "args": {
                            "--originThreads": 1,
                            "--originPort": origin_port,
                            "--originTimeoutMsec": 30000,
                            "--secure": 0,
                            "--responseBodySize": expected_response_size,
                            "--logWarn": null
                        },
                        "endpoints": {
                            "http": {
                                "type": "http",
                                "hostname": "localhost",
                                "port": origin_port
                            }
                        }
                    }
                }
            }
        });

        tf.start(function () {
            var total_remaining_requests = connections * requests_per_connection;

            for (var i = 0; i < connections; ++i) {
                var connection = function (remaining_connection_requests) {
                    var bytes_received = 0;
                    var req = http.request(
                        {
                            hostname: "localhost",
                            port: proxy_port,
                            path: "/foo/bar/baz",
                            method: 'GET',
                            agent: httpAgent
                        },
                        function (res) {
                            log('DEBUG', 'Client received response, status: ' + res.statusCode + ", headers: " + res.rawHeaders);

                            assert.equal(expected_status_code, res.statusCode);

                            res.on('data', function (chunk) {
                                if (process.env['BUILD_TYPE'] !== 'RELEASE') {
                                    for (var i = 0; i < chunk.length; ++i) {
                                        assert.equal("A".charCodeAt(0) + (bytes_received + i) % 26, chunk[i]);
                                    }
                                }

                                bytes_received += chunk.length;
                                assert(expected_response_size >= bytes_received);
                                if (bytes_received === expected_response_size) {
                                    // End HTTP transaction
                                    --total_remaining_requests;
                                    --remaining_connection_requests;

                                    if (0 < remaining_connection_requests) {
                                        connection(remaining_connection_requests);
                                        return;
                                    }

                                    if (0 === total_remaining_requests) {
                                        tf.stop(done);
                                    }
                                }
                            });
                        });

                    req.on('error', function (e) {
                        // Fail HTTP transaction
                        tf.stop(done);
                        assert.fail(e, null, e.toString());
                    });

                    req.end();
                };
                connection(requests_per_connection);
            }
        });
    });


    it('Node.js client to Everscale origin (HTTP 1.1)', function (done) {
        var origin_port = 8080;
        var expected_status_code = 200;
        var expected_response_size = 1024;

        var tf = framework.tf_new({
            log_cb: log,
            config: {
                "servers": {
                    "Everscale origin": {
                        "type": "async process",
                        "path": root_dir + "/data-plane/origin/http-origin",
                        "args": {
                            "--originThreads": 1,
                            "--originPort": origin_port,
                            "--originTimeoutMsec": 30000,
                            "--secure": 0,
                            "--responseBodySize": expected_response_size,
                            "--logWarn": null
                        },
                        "endpoints": {
                            "http": {
                                "type": "http",
                                "hostname": "localhost",
                                "port": origin_port
                            }
                        }
                    }
                }
            }
        });

        tf.start(function () {
            var total_remaining_requests = connections * requests_per_connection;

            for (var i = 0; i < connections; ++i) {
                var connection = function (remaining_connection_requests) {
                    var bytes_received = 0;
                    var req = http.request(
                        {
                            hostname: "localhost",
                            port: origin_port,
                            path: "/foo/bar/baz",
                            method: 'GET',
                            agent: httpAgent
                        },
                        function (res) {
                            log('DEBUG', 'Client received response, status: ' + res.statusCode + ", headers: " + res.rawHeaders);

                            assert.equal(expected_status_code, res.statusCode);

                            res.on('data', function (chunk) {
                                if (process.env['BUILD_TYPE'] !== 'RELEASE') {
                                    for (var i = 0; i < chunk.length; ++i) {
                                        assert.equal("A".charCodeAt(0) + (bytes_received + i) % 26, chunk[i]);
                                    }
                                }

                                bytes_received += chunk.length;
                                assert(expected_response_size >= bytes_received);
                                if (bytes_received === expected_response_size) {
                                    // End HTTP transaction
                                    --total_remaining_requests;
                                    --remaining_connection_requests;

                                    if (0 < remaining_connection_requests) {
                                        connection(remaining_connection_requests);
                                        return;
                                    }

                                    if (0 === total_remaining_requests) {
                                        tf.stop(done);
                                    }
                                }
                            });
                        });

                    req.on('error', function (e) {
                        // Fail HTTP transaction
                        tf.stop(done);
                        assert.fail(e, null, e.toString());
                    });

                    req.end();
                };
                connection(requests_per_connection);
            }
        });
    });

    it('Node.js client to Everscale origin (HTTPS 1.1)', function (done) {
        var origin_port = 8443;
        var expected_status_code = 200;
        var expected_response_size = 1024;

        var tf = framework.tf_new({
            log_cb: log,
            config: {
                "servers": {
                    "Everscale origin": {
                        "type": "async process",
                        "path": root_dir + "/data-plane/origin/http-origin",
                        "args": {
                            "--originThreads": 1,
                            "--originPort": origin_port,
                            "--originTimeoutMsec": 30000,
                            "--secure": 1,
                            "--responseBodySize": expected_response_size,
                            "--caCertPath": ca_path,
                            "--serverCertPath": root_dir + "/base/tests/server.crt",
                            "--serverKeyPath": root_dir + "/base/tests/server.key",
                            "--logWarn": null,
                            agent: httpsAgent
                        },
                        "endpoints": {
                            "http": {
                                "type": "http",
                                "hostname": "localhost",
                                "port": origin_port
                            }
                        }
                    }
                }
            }
        });

        tf.start(function () {
            var total_remaining_requests = connections * requests_per_connection;

            for (var i = 0; i < connections; ++i) {
                var connection = function (remaining_connection_requests) {
                    var bytes_received = 0;
                    var req = https.request(
                        {
                            hostname: "localhost",
                            servername: "test.server.everscale.com",
                            port: origin_port,
                            path: "/foo/bar/baz",
                            method: 'GET',
                            ca: ca_bytes,
                            // key: fs.readFileSync(root_dir + "/base/tests/client.key"),
                            // cert: fs.readFileSync(root_dir + "/base/tests/client.crt"),
                            checkServerIdentity: function (host, cert) {
                                const err = tls.checkServerIdentity(host, cert);
                                if (err) {
                                    log('ERROR', 'TLS handshake failed: ' + err);
                                    return err;
                                }
                            }
                        },
                        function (res) {
                            log('DEBUG', 'Client received response, status: ' + res.statusCode + ", headers: " + res.rawHeaders);

                            assert.equal(expected_status_code, res.statusCode);

                            res.on('data', function (chunk) {
                                if (process.env['BUILD_TYPE'] !== 'RELEASE') {
                                    for (var i = 0; i < chunk.length; ++i) {
                                        assert.equal("A".charCodeAt(0) + (bytes_received + i) % 26, chunk[i]);
                                    }
                                }

                                bytes_received += chunk.length;
                                assert(expected_response_size >= bytes_received);
                                if (bytes_received === expected_response_size) {
                                    // End HTTP transaction
                                    --total_remaining_requests;
                                    --remaining_connection_requests;

                                    if (0 < remaining_connection_requests) {
                                        connection(remaining_connection_requests);
                                        return;
                                    }

                                    if (0 === total_remaining_requests) {
                                        tf.stop(done);
                                    }
                                }
                            });
                        });

                    req.on('error', function (e) {
                        // Fail HTTP transaction
                        tf.stop(done);
                        assert.fail(e, null, e.toString());
                    });

                    req.end();
                };
                connection(requests_per_connection);
            }
        });
    });

    it('Node.js client to Node.js origin (HTTP 1.1)', function (done) {
        var hostname = 'localhost';
        var origin_port = 8080;
        var expected_status_code = 200;
        var num_chunks = 10;
        var chunk_size_bytes = 1024;
        var expected_response_size = num_chunks * chunk_size_bytes;
        var chunk_byte_value = 99;

        var tf = framework.tf_new({
            log_cb: log,
            config: {
                "servers": {
                    "Node origin": {
                        "type": "node origin",
                        "path": "./origin.js",
                        "actions": {
                            "GET": {
                                "/foo/bar/baz": {
                                    "status_code": expected_status_code,
                                    "headers": {},
                                    "delay_first_chunk_millis": 0,
                                    "chunk_size_bytes": chunk_size_bytes,
                                    "num_chunks": num_chunks,
                                    "delay_between_chunk_millis": 0,
                                    "chunk_byte_value": chunk_byte_value
                                }
                            }
                        },
                        "endpoints": {
                            "http": {
                                "type": "http",
                                "hostname": hostname,
                                "port": origin_port
                            }
                        }
                    }
                }
            }
        });

        tf.start(function () {
            var total_remaining_requests = connections * requests_per_connection;

            for (var i = 0; i < connections; ++i) {
                var connection = function (remaining_connection_requests) {
                    var bytes_received = 0;
                    var req = http.request(
                        {
                            hostname: "localhost",
                            port: origin_port,
                            path: "/foo/bar/baz",
                            method: 'GET',
                            agent: httpAgent
                        },
                        function (res) {
                            log('DEBUG', 'Client received response, status: ' + res.statusCode + ", headers: " + res.rawHeaders);

                            assert.equal(expected_status_code, res.statusCode);

                            res.on('data', function (chunk) {
                                bytes_received += chunk.length;
                                assert(expected_response_size >= bytes_received);
                                if (bytes_received === expected_response_size) {
                                    // End HTTP transaction
                                    --total_remaining_requests;
                                    --remaining_connection_requests;

                                    if (0 < remaining_connection_requests) {
                                        connection(remaining_connection_requests);
                                        return;
                                    }

                                    if (0 === total_remaining_requests) {
                                        tf.stop(done);
                                    }
                                }
                            });
                        });

                    req.on('error', function (e) {
                        // Fail HTTP transaction
                        tf.stop(done);
                        assert.fail(e, null, e.toString());
                    });

                    req.end();
                };
                connection(requests_per_connection);
            }
        });
    });

    it('Everscale client to Everscale origin (HTTP 1.1)', function (done) {
        var origin_port = 8080;
        var response_size = 1024;

        var tf = framework.tf_new({
            log_cb: log,
            config: {
                "servers": {
                    "Everscale origin": {
                        "type": "async process",
                        "path": root_dir + "/data-plane/origin/http-origin",
                        "args": {
                            "--originThreads": 3,
                            "--originPort": origin_port,
                            "--originTimeoutMsec": 30000,
                            "--secure": 0,
                            "--responseBodySize": response_size,
                            "--logWarn": null
                        },
                        "endpoints": {
                            "http": {
                                "type": "http",
                                "hostname": "localhost",
                                "port": origin_port
                            }
                        }
                    },
                    "Everscale loadgen": {
                        "type": "sync process",
                        "path": root_dir + "/data-plane/loadgen/http-loadgen",
                        "args": {
                            "--clientThreads": 3,
                            "--connections": connections,
                            "--requestsPerConnection": requests_per_connection,
                            "--reuseConnections": 1,
                            "--destinationAddress": "127.0.0.1",
                            "--destinationPort": origin_port,
                            "--clientTimeoutMsec": 30000,
                            "--secure": 0,
                            "--responseBodySize": response_size,
                            "--logWarn": null
                        }
                    }
                }
            }
        });

        tf.start(function (result) {
            assert.equal(0, result.status);
            tf.stop(done);
        });
    });
});
