#!/bin/bash -e
# setup postgre 9.3 stuffs
#

wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
sudo sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt/ precise-pgdg main" >> /etc/apt/sources.list.d/postgresql.list'

sudo apt-get update -qq
sudo apt-get install -q -y postgresql-9.3-postgis-2.1 postgresql-9.3-postgis-scripts postgresql-9.3 postgresql-client-9.3 postgresql-server-dev-9.3 postgis
