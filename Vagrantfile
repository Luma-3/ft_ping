# -*- mode: ruby -*-
# vi: set ft=ruby :


Vagrant.configure("2") do |config|

  config.vm.box = "debian/bullseye64"

  config.vm.synced_folder ".", "/vagrant_data"

  config.ssh.forward_agent = true

  config.vm.provision "shell", inline: <<-SHELL
    apt-get -y install rsync inetutils-ping
    apt-get update 
    apt install -y make gcc
  SHELL

end
