# ClimbSimArduino
このプロジェクトはZwift上での勾配を固定ローラーで再現することを目的としています。
https://www.instructables.com/id/Open-Bicycle-Grade-Simulator-OpenGradeSIM/
にインスパイアされています。

以下のような接続を想定しています。
```
Zwift on PC -> Zwift Companion -> 本機 -> TacxNeo
```
現在はTacxNeoとの接続を前提としています。
今後のアップデートで単体で使用できるようにするつもりです。  
本機とZwift Companion及びTacxNeoとの接続はBLE経由です。
現在PC上のZwiftからBLEでスマートトレーナーに接続することができないため、スマートフォンのZwift Companionアプリを経由する必要があります。


## 必要なもの
URLは参考です。
* リニアアクチュエータ: 12V 750N 可動範囲250mm程度
  * ポテンショメータ付き(https://ja.aliexpress.com/item/33048985885.html)
  * ポテンショメータ無し(https://ja.aliexpress.com/item/32815426262.html)
* モータドライバ: L298N(https://www.amazon.co.jp/dp/B07J27XX38)
* LLC: 3.3v-5v(https://www.amazon.co.jp/dp/B081RH1P4L)
* MCU: ESP32(https://www.amazon.co.jp/dp/B07DKXS9TT)
* 距離センサ(ポテンショメータ付きリニアアクチュエータの場合は不要): VL53L0X(https://www.amazon.co.jp/dp/B07WK1FRZG)
* ACアダプタ: 12V3A(https://www.amazon.co.jp/dp/B07P84CQPC)

## 回路図
![circuit](https://user-images.githubusercontent.com/6167596/81029511-3575d080-8ec0-11ea-80df-98f50e1db5d1.png)


## 使い方


## 未実装な部分
* 単体動作対応
* 再接続に難がある
* LCD表示(現在の勾配でも表示しようかな)
