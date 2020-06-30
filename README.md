# study ![C/C++ CI](https://github.com/teruteru128/study/workflows/C/C++%20CI/badge.svg)

A garbage dump repository created for studying Teruteru.

## TODO

- 正規表現
- MySQLクライアント
- マルチスレッド(pthreads)
- P2P地震情報プロトコル実装(P2P地震情報のLinux向けC言語実装)作成
    - P2Pノード
        - サーバー＆クライアント
    - 可変長文字列リストもしくはキュー
    - [パケット(プロトコル)構築](https://github.com/p2pquake/epsp-peer-cs/blob/f3cc70fd199806ced719fb9a692ec39b938924ba/Client/Common/Net/Packet.cs#L72)
    - [プロトコルパーサ](https://github.com/p2pquake/epsp-specifications)
- コマンドライン引数の解析

## Dependencies

- libcurl4-gnutls-dev 7.58.0-2ubuntu3.8
- libgmp-dev 2:6.1.2+dfsg-2
- libsodium-dev 1.0.16-2 
- libxmlrpc-core-c3-dev 1.33.14-8build1
- uuid-dev 2.31.1-0.4ubuntu3.6
- cuda-10-2 10.2.89-1
- libupnp-dev 1:1.6.24-4

## このリポジトリに含む~~クソ~~機能

- ロケールに関するテスト実装。i18nテスト実装的な？
- teamspeakのID セキュリティレベル改善ツール。今後使う予定なし。
- コラッツの問題に関する実験的実装。
- 各種暗号系プリミティブを実装しようとした残骸。未完成。
    - AES
    - SHA-1
    - SHA-2
    - SHA-3
    - MD-5
- コマンドライン引数解析ライブラリを作ろうとした残骸。未完成。
- Base64 encoder/decoder。確か2箇所に実装があった気がする。
- 16進数文字列パーサに至ってはもう何箇所にあるのか
- ネットワーク関係の実験コマンド。
    - ブロードキャスト送信ツール
    - UDPパケット送信ツール
    - UDPパケット受信ツール
    - sntpクライアント
- bitmessage CLI的な何か。
    - スパムツールか何かで？
- bitsetを実装しようとした残骸。
- pybitmessage xml-rpcライブラリの残骸。やっぱり未完成。
- BitMessage向けのアドレスを生成するツール。
- 棒読みちゃんリモート読み上げツール。パラメータをリトルエンディアンで実装するのやめてくれよ……
- RSA計算ツールなど
    - ファイルから改行区切りで16進数整数を2つ読み出して乗算して返すだけのツール
- iconvを薄くラップするライブラリ() iconvってなんでこんなにめんどくさいの？
- 某サイトで配布されていた暗号化/復号化()ツールをC言語で実装したライブラリ
- CUDA向けHello World
- CUDA向けメモリ確保テスト。なお実行できる環境を持っていない模様
- curl呼び出しテスト
- きたない台詞を吐き出すクソツール。
    - ああああああああ！ﾌﾞ
    - やったぜ。
    - [ン゛ボ](https://twitter.com/tukushiA/status/844873480805859330)[ップ](https://w.atwiki.jp/aniwotawiki/pages/38145.html#id_73c21301)
    - ブッチッパ！
- 乱数テスト
- Dragon曲線描画ツール。Illustratorじゃ表示できねえぞこんなん！
- secp256k1鍵ペアを大量に生成するツール。
- ファイルから既存の素数を読み出してRSA鍵に変換するツール。
- P2P地震情報のLinux向け実装を作ろうとした残骸。
- 環境変数を取得して見るテスト。
- [ファルコン☆チンパ](https://twitter.com/Fal_conpunch)をより良い乱数で実装しようとした何か。あのbotの乱数偏りがひどすぎるんですって！
- 「Linuxとpthreadによるマルチスレッドプログラミング入門」より引用したマルチスレッドのHello World
- FizzBazz
- 世界のナベアツ
- java.util.RandomのC言語実装。Minecraftのスライムチャンクを探すのに使った。
- 符号なし64bit整数を10進数文字列のchar配列に変換するライブラリとベンチマーク。TeamSpeakのID セキュリティレベル改善に使う
- libsodiumの鍵交換ライブラリを使ってみるテスト。これをJava側との通信に使えたらなあ……
- リスト型を実装しようとした残骸。
- mathライブラリからlog関数を使ってみるテスト。log(2^52)をlog(2)で割って整数になるかどうかとか。
- 無限ループのベンチマークテスト。
- 移動合計の実装しようとした残骸。Java版Minecraftのスライムチャンクの塊を探そうと思った時に作ろうとしたけど挫折した。
- MD5の弱衝突耐性を突破(全ビット0)しようと思って作った残骸。ほぼなにもないね
- どこかから拾ってきたMySQL パスワードブルートフォースツール。どこから拾ってきたんだっけか……？
- int変数1個あたりのNumber of Leading Zeros(NLZ)とNumber of Tailing Zeros(NTZ)を計算するライブラリ(？) ロジックはJavaから持ってきた。
- 謝罪の言葉を連呼するだけのツール。本当に申し訳ありませんでした。
- TCPポートスキャナーを作ろうとした残骸。
- 素数を判定したり素数を探索したり階乗素数を探索したりするツール群。
- struct addrinfoの中身を表示するライブラリ？
- キューを作ろうとした残骸。ばたんきゅ～
- xorshiftを実装したり/dev/randomからバイト列を読み込むためのユーティリティーだったり
- RSA-1024を破ろうと思って作った残骸。そんな簡単じゃないよ
- safe_freeを作ろうとした残骸。もうちょっと真面目に作り込んだらどうなんだい？
- 非線形三項間漸化式のテスト。使う変数型で結果が変わるのやめちくり＾ー
- TCPサーバーを作ろうとした残骸。目的もなしに作ろうとしたのもアレだけど、そもそもサーバーって作るのクソ面倒じゃない？
- unix signalのテスト。
- stdoutとstderrに書き込むユーティリティシェルスクリプト。
- 可変長文字列リスト/配列を作ろうとした残骸。
- 文字列連結を作ろうとした残骸。strcat/strncatで十分。
    - →区切り文字付き文字列連結は？
    - JavaのStringJoiner的なライブラリを目指そうとしてたんだっけ？
- 3立方数の和(Sum of three cubes)の検算ツール
- TeXのHello World。あれがTeXなのかLaTeXなのかよくわかってない。~~ﾌﾌﾌ……TeX!~~
- Tweetクライアントを作ろうとした残骸ファイル。
- UPNPを実装しようとした残骸。このファイルいつから触ってないんだっけ？
- NicoNamaCommentViewerのUserSettingを変換するために作ろうとしたXSLファイルの残骸。
- OpenSSL EVP APIを使ってみるテスト。
    - EVP_Digest

## かつて存在したクソ

- Fork爆弾
