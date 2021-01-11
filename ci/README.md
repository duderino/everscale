# Build & publish

docker build --rm -f build-image -t jtblatt/everscale-build:01-10-21 .
docker push jtblatt/everscale-build:01-10-21

# Interactive test

docker run -it jtblatt/everscale-build:01-10-21
git clone https://github.com/duderino/everscale.git
cd everscale
cmake . && make && make test

