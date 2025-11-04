# -*- mode: ruby -*-
# vi: set ft=ruby :


Vagrant.configure("2") do |config|

  config.vm.provider "virtualbox" do |vb|
    vb.memory = "1024"
    vb.cpus = 2
  end

  config.vm.box = "debian/bullseye64"


  config.vm.synced_folder "./", "/vagrant"

  config.ssh.forward_agent = true

  config.vm.provision "shell", inline: <<-SHELL
    apt-get update 
    apt-get -y install rsync inetutils-ping
    apt install -y make gcc valgrind tcpdump docker docker-compose
    usermod -aG docker vagrant
  SHELL

end
