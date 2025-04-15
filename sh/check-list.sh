sudo find / -type f -size +100M -exec ls -lh {} \; | awk '{ print $9 ": " $5 }'
sudo apt clean
sudo du -sh /*

