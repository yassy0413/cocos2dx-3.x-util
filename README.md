# cocos2dx-3.x-util
Cocos2d-x 3.x 系の為の Utility クラス定義

# Qiita
http://qiita.com/yassy

# Javaソースの組み込み
DeviceCamera等、一部の機能を利用するにはJavaソースが必要になります。<br>
例えば以下のように、ビルド対象とするパスをbuild.gradleへ追記すると導入が簡単です。<br>
<br>
**build.gradle**
```
sourceSets.main {
  java.srcDirs = ["../src", "../src/cocos2dx-3.x-util/java"]
}
```

# 公開アプリ
### 早打暗算
* https://itunes.apple.com/jp/app/smarterrarity/id1125820759?l=ja&mt=8




