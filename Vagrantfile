# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  pg_disk = '/Users/verma/work/lod-points/iowa-city-north-liberty-pg.vdi'

  config.vm.box = "precise64"

  config.vm.hostname = "greyhound-dev"
  config.vm.box_url = "http://files.vagrantup.com/precise64.box"

  config.vm.network :forwarded_port, guest: 80, host: 8080

  config.vm.provider :virtualbox do |vb|
	  vb.customize ["modifyvm", :id, "--memory", "2048"]
	  vb.customize ["modifyvm", :id, "--cpus", "2"]   
	  vb.customize ["modifyvm", :id, "--ioapic", "on"]

	  vb.customize ['storageattach', :id, '--storagectl', 'SATA Controller', '--port', 1, '--device', 0, '--type', 'hdd', '--medium', pg_disk]

  end  

  #
  ppaRepos = [
	  "ppa:ubuntugis/ppa",
	  "ppa:ubuntugis/ppa",
	  "ppa:apokluda/boost1.53"
  ]

  # The postgres/gis family of products is not in the list intentionally since they
  # are explicitly installed in one of the scripts
  packageList = [
	  "git",
	  "build-essential",
	  "libjsoncpp-dev",
	  "pkg-config",
	  "redis-server",
	  "cmake",
	  "libflann-dev",
	  "libgdal-dev",
	  "libpq-dev",
	  "libproj-dev",
	  "libtiff4-dev",
	  "haproxy",
	  "libgeos-dev",
	  "python-all-dev",
	  "python-numpy",
	  "libxml2-dev",
	  "libboost1.53-all-dev",
	  "libbz2-dev",
	  "libsqlite0-dev",
	  "cmake-curses-gui",
	  "screen",
	  "libcunit1-dev",
	  "libgeos++-dev"
  ];

  nodeVersion = "0.8.23"
  nodeURL = "http://nodejs.org/dist/v#{nodeVersion}/node-v#{nodeVersion}-linux-x64.tar.gz"
  
  if Dir.glob("#{File.dirname(__FILE__)}/.vagrant/machines/default/*/id").empty?
	  pkg_cmd = ""

	  # provision node, from nodejs.org
	  pkg_cmd << "echo Provisioning node.js version #{nodeVersion}... ; mkdir -p /tmp/nodejs && \
		wget -qO - #{nodeURL} | tar zxf - --strip-components 1 -C /tmp/nodejs && cd /tmp/nodejs && \
		cp -r * /usr && rm -rf /tmp/nodejs ;"

	  pkg_cmd << "apt-get update -qq; apt-get install -q -y python-software-properties; "

	  if ppaRepos.length > 0
		  ppaRepos.each { |repo| pkg_cmd << "add-apt-repository -y " << repo << " ; " }
		  pkg_cmd << "apt-get update -qq; "
	  end

	  # install packages we need
	  pkg_cmd << "apt-get install -q -y " + packageList.join(" ") << " ; "
	  config.vm.provision :shell, :inline => pkg_cmd

	  scripts = [
		  # install post* before going anywhere with scripts
		  "postgis.sh",

		  "startup.sh",
		  "websocketpp.sh",
		  "libgeotiff.sh",
		  "nitro.sh",
		  "hexer.sh",
		  "p2g.sh",
		  "soci.sh",
		  "laszip.sh",
		  "pdal.sh",
		  "pgpointcloud.sh"
	  ];
	  scripts.each { |script| config.vm.provision :shell, :path => "scripts/vagrant/" << script }
  end
end
