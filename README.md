 <img src="https://github.com/sobajiro2/DIY-DRONE/blob/main/4.jpg"><br/>
I aim to make a simple drone using ESP32<br/>
I often see articles about making drones using ESP32 and ARDUINO, etc.<br/>
I thought it might be possible to do it with just ESP32, so I tried making it.<br/>
current situation<br/>
Extract the stick signal from a junk radio with no technical qualifications in japan and input it to the radio's ESP.<br/>
Remove the flight controller of QUATTROX-EYE (same as SYMA X5C)<br/>
Install a flight controller configured with ESP32<br/>
I created and wrote a sketch with arduino IDE<br/>
I use M5STACK-A105 ESP32DOWNLOADER to write the program.<br/>
It flies somehow in rate mode, but it feels like it's being pushed back after operation, which is a future issue.<br/>
Level mode is still under development. Adjustments are being made<br/>
Eventually, I would like to make adjustments and program changes to make it more stable and build my own aircraft.<br/>
Main usage<br/>
Stick left lower left Stick right lower right radio reset (=bind)<br/>
<img src="https://github.com/sobajiro2/DIY-DRONE/blob/main/7.png"><br/>
Stick left fixed at bottom left Stick right move up and down to trim pitch<br/>
Stick left fixed at lower left Stick right move left and right to roll trim<br/>
Stick right fixed at upper right Stick left move left and right to adjust the yaw rim<br/>
Stick left lower right Stick right lower left Calibration and rate/level switching<br/>
<img src="https://github.com/sobajiro2/DIY-DRONE/blob/main/6.png"><br/>
Settings and status can be displayed using the serial port of the ESP for the radio.<br/>
others<br/>
I am using the M5STACK-A105 ESP32DOWNLOADER for the ESP for the radio because the serial interface of the development board was broken, but normally I think there is no problem with the serial interface of the development board.<br/>
I think you can also use a development board for the flight controller.<br/>
In addition, some places use off-the-shelf parts for each part, and the parts may not be optimal.<br/>
It looks like quite high-density wiring, but I was able to make it using heat-resistant wire of about AWG28 and a holding jig.<br/>
It took me about 2 months from the time I came up with the idea to the first post, but it's been a lot of fun as I've had some setbacks and other times I've made great progress.<br/>
Above all, the articles introducing many people who have made their own drones were very encouraging.<br/>
ESP32を使ってシンプルなドローンを作ることを目指しています<br/>
ESP32とARDUINOなどを使ってドローンを作る記事はよく見かけますが<br/>
ESP32だけでもできるのではと思い、作ってみました<br/>
現状<br/>
技適なしジャンク品プロポのスティック信号を取り出してプロポ用ESPへ入力<br/>
QUATTROX-EYE(SYMA X5Cと同じ)のフライトコントローラを取り外して<br/>
ESP32で構成したフライトコントローラを取り付け<br/>
arduino IDEでスケッチを作って書き込みました<br/>
プログラムの書き込みにはM5STACK-A105 ESP32DOWNLOADER　を使っています<br/>
レートモードで何とか飛びますが、操作後の押し戻され感があるのが今後の課題です<br/>
レベルモードは発展途上。調整中です<br/>
ゆくゆくはもっと安定する調整やプログラム変更を行い、機体を自作したいと思っています<br/>
主な使い方<br/>
スティック左　左下　スティック右　右下　プロポリセット（=バインド）<br/>
スティック左　左下固定　スティック右　上下に動かすことにより　ピッチトリム<br/>
スティック左　左下固定　スティック右　左右に動かすことにより　ロールトリム<br/>
スティック右　右上固定　スティック左　左右に動かすことにより　ヨートリム<br/>
スティック左　右下　スティック右　左下　キャリブレーションおよびレート/レベル切り替え<br/>
プロポ用ESPのシリアルポートを使うと設定や状態表示ができます<br/>
その他<br/>
プロポ用ESPは開発ボードのシリアルインターフェースが壊れてしまったのでM5STACK-A105 ESP32DOWNLOADER　を使っていますが　通常なら開発ボードのシリアルインターフェースで問題ないと思います<br/>
フライトコントローラも開発ボードを使ってもいいと思います<br/>
その他、各部ありあわせの部品をつかっているところもあり、最適な部品とは限りません<br/>
かなり高密度配線に見えますがAWG28程度の耐熱電線や保持治具を使って何とか作ることができました<br/>
思い立ってから、最初の書き込みまでは2か月ほどかかりましたが、頓挫したり、ある時は飛躍的に進んだり、とても楽しいです<br/>
なにより、たくさんの自作ドローンを作られた方々の紹介記事がとても励みになりました<br/>
