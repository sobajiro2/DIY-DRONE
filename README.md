ESP32を使ってシンプルなドローンを作ることを目指しています

現状

技適なしジャンク品プロポのスティック信号を取り出してプロポ用ESPへ入力

QUATTROX-EYE(SYMA X5Cと同じ)のフライトコントローラを取り外して

ESP32で構成したフライトコントローラを取り付け

arduino IDEでスケッチを作って書き込みました

レートモードで何とか飛びますが、操作後の押し戻され感があるのが今後の課題です

レベルモードは発展途上。調整中です

ゆくゆくはもっと安定する調整やプログラム変更を行い、機体を自作したいと思っています

主な使い方

スティック左　左下　スティック右　右下　プロポリセット（=バインド）

スティック左　左下固定　スティック右　上下に動かすことにより　ピッチトリム

スティック左　左下固定　スティック右　左右に動かすことにより　ロールトリム

スティック右　右上固定　スティック左　左右に動かすことにより　ヨートリム

スティック左　右下　スティック右　左下　キャリブレーションおよびレート/レベル切り替え

プロポ用ESPのシリアルポートを使うと設定や状態表示ができます
