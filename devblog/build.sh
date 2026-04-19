hugo
rm -rf ../docs
mv public ../docs

sed -i '' 's#"../fonts/#"./fonts/#g' ../docs/styles.css

cp -r ../builder ../docs/builder