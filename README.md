# ClimbSimArduino

このプロジェクトは Zwift 上での勾配を固定ローラーで再現することを目的としています。
https://www.instructables.com/id/Open-Bicycle-Grade-Simulator-OpenGradeSIM/
にインスパイアされています。

以下のような接続を想定しています。

```
Zwift on PC -> Zwift Companion -> 本機 -> TacxNeo
```

本機と Zwift Companion 及び TacxNeo との接続は BLE 経由です。  
現在 PC 上の Zwift から BLE でスマートトレーナーに接続することができないため、スマートフォンの Zwift Companion アプリを経由する必要があります。

## 必要なもの

URL は参考です。

- リニアアクチュエータ: 12V 750N 可動範囲 250mm 程度 ※ 750N だと少しトルク不足かも
  - ポテンショメータ付き(https://ja.aliexpress.com/item/33048985885.html)
  - ポテンショメータ無し(https://ja.aliexpress.com/item/32815426262.html)
- モータドライバ: L298N(https://www.amazon.co.jp/dp/B07J27XX38)
- LLC: 3.3v-5v(https://www.amazon.co.jp/dp/B081RH1P4L)
- MCU: ESP32(https://www.amazon.co.jp/dp/B07DKXS9TT)
- 距離センサ(ポテンショメータ付きリニアアクチュエータの場合は不要): VL53L0X(https://www.amazon.co.jp/dp/B07WK1FRZG)
- AC アダプタ: 12V3A(https://www.amazon.co.jp/dp/B07P84CQPC)

## 回路図

![circuit](https://user-images.githubusercontent.com/6167596/81029511-3575d080-8ec0-11ea-80df-98f50e1db5d1.png)

## 開発

- VSCode をインストールする
- VSCode の拡張機能から PlatformIO をインストールする
- PIO Home の Libraries から「Adafruit_SSD1306」, 「Adafruit_VL53L0X」を検索してインストールする
- 必要に応じてコードを変更する
- Build -> Upload -> Serial Monitor

### コードに変更が必要な箇所

- VL53L0X を使用する場合、`#define SENSOR_TYPE_POTENTIOMETER`をコメントアウトしてください
- VL53L0X を使用する場合、デフォルトではセンサから 10cm を斜度 0 としています。変更する場合は main.c の`#define SLOPE_BETA (100)`を編集してください(mm 単位)
- ポテンショメータ等のアナログセンサを使用する場合、`SLOPE_ALPHA`と`SLOPE_BETA`の値を読み取り値に応じて変更してください
- Zwift の難易度設定に応じて、`ZWIFT_DIFFICULTY`を変更してください。デフォルト設定の場合は`1`で ok です

## 使い方

1. PC アプリを起動する
2. Zwift Companion アプリを起動する
3. PC アプリの接続画面で右上の設定から、「Zwift コンパニオンアプリを使う」を選択する
4. 本機を起動する
5. スマートトレーナーに「Tacx Neo dummy ###」を選択する ※ このとき、パワーやケイデンスセンサーに本機(Tacx Neo dummy)を選択しないでください
6. TacxNeo が起動している場合自動的に接続します(Tacx 系の他のトレーナーも接続できるかもしれません。それ以外はおそらく接続できません

※ ERG モードには対応していません。使いたい場合は直接トレーナーと接続するようにしてください  
※ Zwift 以外に Tacx の Training アプリからも接続することもできます。
