var assert = require("assert");
var client = require('http').request;
var framework = require('./framework.js');

var log = function (thresh, data) {
    console.error(thresh + ': ' + data);
};

var root_dir = "../..";

describe('Example', function () {
    var tf = null;

    after(function (done) {
        if (tf) {
            tf.stop(done);
        }
    });

    it('origin-http', function (done) {
        var origin_port = 8080;
        var expected_status_code = 200;
        var expected_response_size = 1024;

        tf = framework.tf_new({
            log_cb: log,
            config: {
                "test": {
                    "iterations": 1,
                    "concurrency": 1
                },
                "servers": {
                    "origin": {
                        "type": "executable",
                        "path": root_dir + "/data-plane/origin/http-origin",
                        "args": {
                            "--originThreads": 1,
                            "--port": origin_port,
                            "--originTimeoutMsec": 30000,
                            "--secure": 0,
                            "--responseBodySize": expected_response_size,
                            "--caCertPath": root_dir + "/base/tests/ca.crt",
                            "--serverCertPath": root_dir + "/base/tests/server.crt",
                            "--serverKeyPath": root_dir + "/base/tests/server.key",
                            "--logInfo": null
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
            var bytes_received = 0;
            var req = client(
                {
                    hostname: "localhost",
                    port: origin_port,
                    path: "/foo/bar/baz",
                    method: 'GET'
                },
                function (res) {
                    log('DEBUG', 'STATUS: ' + res.statusCode);
                    log('DEBUG', 'HEADERS: ' + JSON.stringify(res.headers));

                    assert.equal(expected_status_code, res.statusCode);

                    res.on('data', function (chunk) {
                        for (var i = 0; i < chunk.length; ++i) {
                            assert.equal("A".charCodeAt(0) + (bytes_received + i) % 26, chunk[i]);
                        }

                        bytes_received += chunk.length;
                        assert(expected_response_size >= bytes_received);
                        if (bytes_received == expected_response_size) {
                            // End test (or timeout and fail)
                            done();
                        }
                    });
                });

            req.on('error', function (e) {
                assert.fail(e, null, 'socket error');
            });

            req.end();
        });
    });
});
