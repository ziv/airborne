hugo
rm -rf ../docs
mv public ../docs
sed -i '' 's#"../fonts/#"./fonts/#g' ../docs/styles.css
cd ..
git add .
git commit -m "Update docs"
git push origin main