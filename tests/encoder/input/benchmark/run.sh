# btormc = btormc
mkdir -p out

for file in $(ls | grep -E ".btor$")
do
# echo $file
btormc -kmax 240  $file > out/$file.txt
done