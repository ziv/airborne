hugo
rm -rf ../docs
mv public ../docs
sed -i '' 's#"../fonts/#"./fonts/#g' ../docs/styles.css
cd ..
cp -r ./builder ./docs/builder
git add .
git commit -m "Update docs"
git push origin main