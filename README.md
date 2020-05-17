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

## 開発
* VSCodeをインストールする
* VSCodeの拡張機能からPlatformIOをインストールする
* PIO HomeのLibrariesからライブラリ(Adafruit_SSD1306, Adafruit_VL53L0X)をインストールする
* Build -> Upload -> Serial Monitor

デフォルトではセンサから10cmを斜度0としています。変更する場合はmain.cの```#define SLOPE_BETA (100)```を編集してください(mm単位)


## 使い方
1. PCアプリを起動する
2. Zwift Companionアプリを起動する
3. PCアプリの接続画面で右上の設定から、「Zwiftコンパニオンアプリを使う」を選択する
4. 本機起動する
5. スマートトレーナーに「Tacx Neo dummy ###」を選択する
※ このとき、パワーやケイデンスセンサーに本機(Tacx Neo dummy)を選択しないでください。
※ TacxNeoが起動している場合自動的に接続します。(Tacx系の他のトレーナーも接続できるかもしれません。それ以外はおそらく接続できません


※ TacxのTrainingアプリからも接続することもできますが、アプリで設定した斜度の倍の値が設定されます。Zwiftが実際の傾斜値の半分の値を送信しているようだったので倍にしてます。
※ERGモードには対応していません。使いたい場合は直接トレーナーと接続するようにしてください。
