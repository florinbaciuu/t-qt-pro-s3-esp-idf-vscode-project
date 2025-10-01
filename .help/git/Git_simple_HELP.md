# 📘 Ghid Git


### GIT:
```
https://github.com/florinbaciuu/lilygo-tqtpros3-esp-idf-project.git
```
---
#### Inițializează Git general
```
git config --global user.name "florinbaciuu"                    # Numele tău pentru toate commiturile
git config --global user.email "baciuaurelflorin@gmail.com"     # Emailul asociat contului GitHub
git config --global init.defaultBranch main
```
---
#### Inițializează Git in folder
```
git init
```
---
#### Fă commit:
```
git add .
git commit -m "Primul commit"
```
---
#### Creează branch main:
```
git branch -M main
```
---
#### Adăugare submodul Git
```
git submodule add https://github.com/florinbaciuu/ESP32_Resource_Monitor.git lib/ESP32_Resource_Monitor
```
---
#### Setează remote și trimite prima data:
```
git remote add origin https://github.com/florinbaciuu/lilygo-tqtpros3-esp-idf-project.git
git push --set-upstream origin main
git push origin
```
---
#### Lucreaza apoi cu :
```
git add .
git commit -m "commit"
git push origin
```
---


code `code`