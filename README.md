Development of landing site for stouza.ru / ястоюза.рф

## Prerequisites

```
sudo apt install git g++ cmake libmicrohttpd-dev certbot
```

## Buidling

Clone the sources:

```
git clone https://github.com/stouza/stouza.ru.git
```

Create SSL certificates:

```
cd stouza.ru
mkdir ssl
cd ssl
sudo certbot certonly -d $(hostname) --standalone --agree-tos -m marcusmae@gmail.com
sudo cp /etc/letsencrypt/live/stouza.ru/privkey.pem id_rsa.stouza
sudo cp /etc/letsencrypt/live/stouza.ru/fullchain.pem id_rsa.stouza.crt
cd ..
```

Build the server:

```
mkdir build
cd build
cmake ..
make -j12
```

## Deployment

Deploy the server eiter on an arbitrary non-default port for development, or on port 443 to have SSL support and redirection from HTTP to HTTPS:

```
sudo ./stouza 443
```

