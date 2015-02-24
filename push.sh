#!/bin/bash
# Script to deploy to vagrant dev instance
rsync -azPr -e "ssh -p 2222 -i /Users/rfaulkner/.vagrant.d/insecure_private_key" . vagrant@127.0.0.1:/home/vagrant/databayes > push.log