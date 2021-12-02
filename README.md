# study ![cmake](https://github.com/teruteru128/study/workflows/Release/badge.svg)![cmake](https://github.com/teruteru128/study/workflows/cmake/badge.svg)

A garbage dump repository created for studying Teruteru.

## ライセンスについて(About license)

他人のWebサイトからコピペしたソースコードが多数含まれているためGPLを適用することができるのか正直わかりません。ご了承ください

## TODO

- コマンドライン引数の解析
- ファイル名の区切りをハイフンかアンダースコアのどちらかに統一する
- TODO: signalfdのサンプル作成
- 対称鍵暗号,EVP_CIPHER:https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
- 認証付き暗号:https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
- エンベロープ暗号化(ハイブリッド暗号？):https://wiki.openssl.org/index.php/EVP_Asymmetric_Encryption_and_Decryption_of_an_Envelope
- 署名と検証,EVP_DigestSign:https://wiki.openssl.org/index.php/EVP_Signing_and_Verifying
- メッセージダイジェスト:https://wiki.openssl.org/index.php/EVP_Message_Digests
- 鍵合意(鍵交換),EVP_PKEY_CTX,EVP_PKEY_derive:https://wiki.openssl.org/index.php/EVP_Key_Derivation
- EVPとRSAを使って署名/検証
  - EVP+ed25519(EdDSA)
  - EVP+X25519(EdDH)
  - EVP+ChaCha20/Poly1305
  - メッセージ認証符号 (OpenSSL 3～),EVP_MAC_new_ctx
  - 鍵導出関数,EVP_KDF:https://wiki.openssl.org/index.php/EVP_Key_Derivation
- strsep,トークン分割(空フィールド対応版)
- versionsort
- strverscmp
- alphasort
- tor geoip file 読み込み関数
- geoip_load_file
- https://youtu.be/MCzo6ZMfKa4
- ターミュレーター
- getopt
- getsubopt

- regex.h
  - 最左最短一致
  - 否定先読み
  - 強欲な数量子

- 複数スレッドをpthread_cond_tで止めてメインスレッドでtimerfdを使って指定時刻まで待ち、pthread_cond_broadcastで一斉に起動する

- ファイルからGMPのmpzに整数を読み込んだりOpenSSLのBIGNUMに整数を読み込んだり乱数を読み込んだりを共通化したい

- TODO: #43 既存の鍵からbitmessage用アドレスを探索する
- P2P地震情報の実装を作る
- P2P地震情報 ピア接続受け入れ＆ピアへ接続

  - 標準入力と標準出力を別スレッドで行うアプリ
  - リクエストを投げると適当なデータを投げ返す簡単なサーバープログラム
  - リクエスト長さは8バイトに対応

  - --help
  - --version
  - --server-mode
    - フォアグラウンドで起動
  - --daemon-mode
    デーモン化処理付きでバックグラウンドで起動
  - P2P地震情報プロトコル実装(P2P地震情報のLinux向けC言語実装)作成
  - P2Pノード
    - サーバー＆クライアント
  - 可変長文字列リストもしくはキュー
  - [パケット(プロトコル)構築](https://github.com/p2pquake/epsp-peer-cs/blob/f3cc70fd199806ced719fb9a692ec39b938924ba/Client/Common/Net/Packet.cs#L72)
  - [プロトコルパーサ](https://github.com/p2pquake/epsp-specifications)
  - Rustがいいかも？
  - まずスレッドがいくつ必要になるかも把握してないんだが
    - メインスレッドに何をさせるか、させないか
- BitMessageの実装を作る
  - Rustがいいかも？
- 82589934bit 素数候補を検証する
  - ミラーラビン素数判定法によるロジックそのものをマルチスレッド化する必要があるかもしれない

## Dependencies

- libcurl4-gnutls-dev 7.58.0-2ubuntu3.8
- libgmp-dev 2:6.1.2+dfsg-2
- libsodium-dev 1.0.16-2
- libxmlrpc-core-c3-dev 1.33.14-8build1
- uuid-dev 2.31.1-0.4ubuntu3.6
- libupnp-dev 1:1.6.24-4
- libjson-c-dev

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
  - RSA-1024を破ろうと思って作った残骸。そんな簡単じゃないよ
  - ファイルから既存の素数を読み出してRSA鍵に変換するツール。
  - 素数探索の開始地点を生成するツール
  - マルチスレッドで素数を探索するツール
- iconvを薄くラップするライブラリ() iconvってなんでこんなにめんどくさいの？
- 某サイトで配布されていた暗号化/復号化()ツールをC言語で実装したライブラリ？
- curl呼び出しテスト
- きたない台詞をひり出すクソツール。:poop:
  - ああああああああ！ﾌﾞ
  - やったぜ。
  - [ン゛ボ](https://twitter.com/tukushiA/status/844873480805859330)[ップ](https://w.atwiki.jp/aniwotawiki/pages/38145.html#id_73c21301)
  - ブッチッパ！
- 乱数テスト
- Dragon曲線描画ツール。Illustratorじゃ表示できねえぞこんなん！
- secp256k1鍵ペアを大量に生成するツール。
- P2P地震情報のLinux向け実装を作ろうとした残骸。
- 環境変数を取得して見るテスト。
- [ファルコン☆チンパ](https://twitter.com/Fal_conpunch)をより良い乱数で実装しようとした何か。あのbotの乱数偏りがひどすぎるんですって！
- 「Linuxとpthreadによるマルチスレッドプログラミング入門」より引用したマルチスレッドのHello World
- FizzBazz
- 世界のナベアツ
- java.util.RandomのC言語実装
  - Minecraftのスライムチャンクを探すのに使った。
- 符号なし64bit整数を10進数文字列のchar配列に変換するライブラリとベンチマーク
  - TeamSpeakのID セキュリティレベル改善に使った
- libsodiumの鍵交換ライブラリを使ってみるテスト。これをJava側との通信に使えたらなあ……
- リスト型を実装しようとした残骸。
- mathライブラリからlog関数を使ってみるテスト。log(2^52)をlog(2)で割って整数になるかどうかとか。
- 無限ループのベンチマークテスト。
- 移動合計の実装しようとした残骸。Java版Minecraftのスライムチャンクの塊を探そうと思った時に作ろうとしたけど挫折した。
- MD5の弱衝突耐性を突破(全ビット0)しようと思って作った残骸。ほぼなにもないね
- どこかから拾ってきたMySQL パスワードブルートフォースツール。どこから拾ってきたんだっけか……？
- int変数1個あたりのNumber of Leading Zeros(NLZ)とNumber of Tailing Zeros(NTZ)を計算するライブラリ(？) ロジックはJavaから持ってきた。
- 謝罪の言葉を連呼するだけのツール。
  - 本当に申し訳ありませんでした。
- TCPポートスキャナーを作ろうとした残骸。
- 素数を判定したり素数を探索したり階乗素数を探索したりするツール群。
- struct addrinfoの中身を表示するライブラリ？
- キューを作ろうとした残骸
  - ~~ばたんきゅ～~~
- xorshiftを実装したり/dev/randomからバイト列を読み込むためのユーティリティーだったり
- safe_freeを作ろうとした残骸。もうちょっと真面目に作り込んだらどうなんだい？
- 非線形三項間漸化式のテスト。使う変数型で結果が変わるのやめちくり＾ー
- TCPサーバーを作ろうとした残骸
  - 目的もなしに作ろうとしたのもアレだけど、そもそもサーバーって作るのクソ面倒じゃない？
- unix signalのテスト。
- stdoutとstderrに書き込むユーティリティシェルスクリプト。
- 可変長文字列リスト/配列を作ろうとした残骸。
- 文字列連結を作ろうとした残骸。strcat/strncatで十分。
  - →区切り文字付き文字列連結は？
  - JavaのStringJoiner的なライブラリを目指そうとしてたんだっけ？
- 3立方数の和(Sum of three cubes)の検算ツール？
- TeXのHello World。あれがTeXなのかLaTeXなのかよくわかってない。~~ﾌﾌﾌ……TeX!~~
- Tweetクライアントを作ろうとした残骸ファイル。
- UPNPを実装しようとした残骸。~~このファイルいつから触ってないんだっけ？~~
- NicoNamaCommentViewerのUserSettingを変換するために作ろうとしたXSLファイルの残骸。
- OpenSSL EVP APIを使ってみるテスト。
  - EVP_Digest

## かつて存在したクソ

- Fork爆弾

## BM

- https://12factor.net/ja/
- 簡単に立てたり落としたりできるDockerにまとめたい

### 設定ファイル

- 秘密鍵

### 永続化DB

- ノード一覧
- 受信した公開鍵
- メッセージ
  - 受信ボックス
  - 送信ボックス
- 受信オブジェクト
  - 処理済みカラムもつけたい

### キャッシュ

