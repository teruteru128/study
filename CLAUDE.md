# study リポジトリの現状(2026-08-17時点)

`study`はREADME.mdが自称する通り雑多な個人リポジトリで、暗号プリミティブの未完成実装・i18nテスト・Minecraft関連ツールなど無関係な内容を多数含む。**現在アクティブに開発しているのは以下の2つ(素数探索とbitmessageアドレス探索)**で、それ以外の大半は過去の実験や中断した作業。ただし「古い＝再利用不可」とは限らない。実装済みの暗号プリミティブやユーティリティが今後別タスクの土台として流用できる可能性はあるので、無関係と決めつけず、既存コードに使えるものがないか一応grep等で確認してから書き始めるとよい。確認範囲はこのリポジトリ本体だけでなく、gitサブモジュール(`libstudy`, `java`, `math-scripts`, `python`)配下も対象に含めること。サブモジュールは`git clone`直後は中身が空なので、`git submodule update --init --recursive`していないと存在に気づけない点に注意。

## 進行中のプロジェクト: 2,097,152bit RSA鍵向け素数探索

- 2つの偶数(`even-number-2097152bit-<UUID>.txt`)それぞれについて、素数を1つ見つけようとしている
  - `037c1901-916f-4ce8-9461-cba9e1f4851f`
  - `49d09838-e81d-470e-a6eb-7157ea24ac6c`
- 既知素数篩(2.7×10^11まで)で候補を約4%まで削り込み済み。**DBは自宅のPostgres**(`jdbc:postgresql://127.0.0.1:5432/primesearch`、接続情報は`java/run-prime-search.sh`にある)。リポジトリ直下の`candidates.sqlite3`は移行前の遺物で、現在は使われていない
- 進捗(2026-09-20時点): 候補572,165件のうち**判定済み911件、素数はまだ0件**。1日あたり約20件。2つの偶数は`candidates`テーブルの`id`列で区別する(`id=251103173751557352`が037c1901、`id=5318918530104379150`が49d09838)
- **1本の素数を見つけるのに期待値で約31,000件**の判定が要る計算(2Mbit奇数が素数である確率2/ln(N)=1/726,818を、篩の残存率4.3%で割ったもの)。1日20件なので**このマシンだけだと1本あたり約4.3年、RSAに必要な2本で8〜9年**。候補プール自体は各偶数に28.5万件あり、期待値では1つの偶数あたり9本前後の素数が埋まっているので枯渇の心配は無い
- 残った候補をGMPの`mpz_probab_prime_p`(BPSW)でMiller-Rabin判定中。1件あたり実測で約7〜15時間かかる、非常に重い探索
- **systemdのuserサービスとして無人稼働中**。`systemctl --user status prime-search.target 'prime-search@*'`で状態確認、`journalctl --user -u prime-search@49d09838.service`でログ確認
- **稼働しているのは`49d09838`だけ(8スレッド)。`037c1901`は意図的に止めている**(`prime-search@037c1901.service`はmasked)。2本同時に動かすのは無理だった。**`49d09838`の探索が終わってから`037c1901`を起動する**こと(`systemctl --user unmask prime-search@037c1901.service`してから起動)。maskedを見て「止まっている、直さなきゃ」と勝手に起動しないこと
- コードを変更したら`./gradlew :prime-search:installDist`してから`systemctl --user restart prime-search.target`しないと反映されない(Gradle経由の起動は`./gradlew --stop`の巻き添えで落ちる事故が起きたため廃止した)
- 詳しい経緯・ハマりどころは`Claude`の自動メモリ(`build_workflow.md`, `gmp_windows_long_gotcha.md`, `java_gmp_binding_mismatch.md`, `even_number_file_format_history.md`, `old_main_pc_broken.md`, `prime_search_systemd_deployment.md`, `gce_postgres_scaleout_plan.md`, `verify_portability_claims_rigorously.md`)を参照

### GCEスケールアウト計画(2026-08-17〜、2026-09-20に**見送り**)

もともとは「自宅マシンだけで回し続けると電気代がかさむため、GCEのスポットインスタンス(c3-standard-88等)を追加投入する」という計画だった。**2026-09-20に実測したところ、この前提が逆だった。**

素数2本ぶんの総額:

| 方式 | 総額 |
|---|---|
| **自宅マシン(電気代)** | **13万〜21万円** |
| GCE スポット | 約151万円 |
| GCE 3年コミット | 約189万円 |
| GCE オンデマンド | 約421万円 |

時間あたりで **GCEスポット22円/時 対 自宅1.7円/時**。`c3-standard-8`は自宅マシンとほぼ同性能(21.4対20判定/日)なので、素直に13倍高いだけだった。**「同じ計算を安く回す」用途にクラウドは向かない。** 費用が出せないため計画は見送り。

クラウドに意味があるのは時間を金で買う場合だけ(自宅1台なら8.5年、GCEスポット10台で約9か月、追加151万円)。詳細な実測値と価格の取得方法は自動メモリ`gce-prime-search-benchmark.md`。

**下地自体は完成しているので、気が変わればすぐ再開できる**: Postgresは`100.79.197.1:5432`(Tailscale)で待ち受けており外部から接続確認済み、`PrimeSearchTask2`にアトミックなclaimと`--stale-hours`があるのでスポット回収にも耐える。gcloudは`~/google-cloud-sdk`に導入済み。

- 進捗管理コードを`java/foreign`・`java/develop`から専用モジュール`java/prime-search`(`com.github.teruteru.primesearch`)へ切り出し済み(java-studyコミット`00366cc2`)。**`PrimeSearch`/`PrimeSearchTask2`/`Result`/`Gmp`facadeは`foreign`ではなく`prime-search`が正**
- DB接続は`SQLiteDataSource`決め打ちから`DriverManager.getConnection(DB_URL)`に統一済み。`DB_URL`のスキーム(`jdbc:sqlite:`/`jdbc:postgresql:`)で自動的にドライバが切り替わる
- 複数マシンの二重着手を防ぐため、候補行のアトミックなclaim(`--stale-hours`オプション、既定24h)を`PrimeSearchTask2`に実装済み
- **完了済み**(2026-09-20に実機で確認): 自宅Postgresサーバー導入、`candidates.sqlite3`からのデータ移行(572,165件がPostgresに入っている)、`run-prime-search.sh`のPostgres/新launcherへの切り替え、Tailscale接続設定(このマシンが`server01` / 100.79.197.1として参加済み)
- **未着手**: GCEインスタンスの実際の構築。`gcloud`コマンドすら入っていない。Tailscaleに参加しているノードもこのマシン1台だけ
- **効くのは台数だけ。** 1件9.5時間という処理速度はほぼ限界で(2Mbitの冪剰余は約200万回の2Mbit二乗算。FFT乗算を使っても理論上10時間前後)、GMPを速くする余地はほとんど無い
- **SMTはほとんど効かない(実測)。** 2Mbitの`mpn_sqr`は物理コア数までは劣化ゼロでスケールするが、そこを超えると1.68倍遅くなる。8スレッドの実効は4.76コア相当で、4スレッドに対して19%しか増えない。**vCPU数を性能と読んではいけない**(`c3-standard-88`の88vCPUは約52コア相当)
- 詳細は自動メモリ`gce_postgres_scaleout_plan.md`

## 進行中のプロジェクト: bitmessageアドレス探索(2026-09-19〜)

先頭に多くのゼロを持つripeのbitmessageアドレスを、既存の公開鍵ファイル群から総当たりで探す。
`/media/teruteru/HD-NRLD-A/避難所/keys/public/publicKeys{0..255}.bin`(各1,090,519,040バイト = 16,777,216鍵 × 65バイト)を使う。

- コマンドは`java/develop`の`addressSearch`(A×A)、`addressSearch2`(2ファイル、8スレッド)、`addressSearch4`(1署名鍵×256ファイル)、`addressSearch5`(1024鍵ずつ、軽い)。**以前は3つとも起動できない状態だった**(picocliの配線漏れ)ので、動かないと思ったらまず`--help`で確認すること
- ripe計算は`java/bmhash`モジュール経由で、`study`本体の`src/rmd160_avx512.c`・`src/sha512_avx512.c`(AVX-512の16レーン実装)を呼ぶ。ビルドは`cmake --build build-Release --target bmhash16`
- ネイティブライブラリの場所はシステムプロパティ`com.github.teruteru.bmhash16.library`に絶対パスを渡す。渡さなければ`BmHash16.isAvailable()`がfalseになりOpenSSLの1件ずつの経路へ自動的に落ちるので壊れはしない。**実行時は必ずログの「16レーン実装: 有効」を確認すること**
- 高速化の結果、`addressSearch4`は1ファイルあたり52.71秒→4.27秒(約12倍)。A×Aの全走査(2^48通り)は3.8年→約141日(素数探索と併走時)になった
- **素数探索とCPUを取り合う。** 物理8コアしかないので、併走させると両方遅くなる。`addressSearch2`には`--threads`オプションがある。`addressSearch`は並列ストリームなので`-Djava.util.concurrent.ForkJoinPool.common.parallelism=N`で絞る
- 詳しい経緯は自動メモリ(`bmhash16-avx512.md`, `native-ripemd160-openssl.md`, `ec-batch-inversion.md`, `bitmessage-address-zero-stripping.md`, `vector-api-rmd160-experiment.md`, `measure-before-hypothesizing.md`)を参照

### 既知の罠

- ~~`./gradlew :foreign:installDist`を実行しないこと~~: **解消済み**。`run-prime-search.sh`が`prime-search`のlauncherを指すようになったため、`foreign`を再ビルドしても本番は壊れない
- **GMPバインディング混在**: `gmp-linux`(Linux向け、64bit、正しい)と`gmp-msys2`(Windows/LLP64向け、`unsigned long`が32bitの`int`として扱われる)の2系統が混在している。`gmp-msys2`を使うコードで`mpz_add_ui`/`mpz_set_ui`/`mpz_fdiv_ui`等に2^31を超える値を渡すとLinux上でもサイレントに壊れる。`prime-search`モジュールの`Gmp`facadeは`gmp-linux`を正しく使っているが、`foreign`に残っている一部の旧コード(`PrimeSearchTask.java`など、未使用の遺物)は今も`gmp-msys2`をimportしている。GMP呼び出しを含むJavaコードを触るときは、どちらのパッケージをimportしているか必ず確認する
- **C側の`src/bmkeysearch*.c`(17本)は現在動かない**: パスが`/mnt/d/keys/...`とWSL時代のハードコードで、公開鍵を64バイト刻み(先頭の0x04を落とした`trimmed`形式)で読むが、その形式のファイルは削除済み。今あるのは65バイト刻み。復活させるなら引数化とtrimmed再生成が要る。なお`libbmhash16`を`target_link_libraries`するだけでAVX-512化できるので、整備さえすればJava側と同じ速度が出る
- **even-numberファイルの10進/16進混在**: 旧1,048,576bit世代(外付けHDD保管)は10進数、現行2,097,152bit世代は16進数。10進ファイルを誤って16進として読んでも構文エラーにならず無音で違う値になる(逆方向は`a`〜`f`混入で即エラーになるため気づきやすい)。C側`load_even_base`とJava側`PrimeSearch`には、a-fを1つも含まないファイルを警告するチェックを追加済み

## 計算資源

- 現在稼働中のマシンはRAM 27GB程度(「ミニコンピュータ」)。Miller-Rabin判定を15スレッド並列で回すとメモリ帯域が奪い合いになり実効速度が半減する現象を確認済み
- 故障中の旧メインPC(RAM 128GB)が自宅にあり、修理すれば計算資源として追加投入できる(型番未確認、修理店に持ち込み待ち)
- **GCE投入は費用が見合わず見送り**(上記参照)。`java/postgres-db-migration-TODO.txt`は初期の移行メモで、移行自体は完了済みなので歴史的資料
- **現実的に効くのは旧メインPCの修理**。クラウドより費用対効果が明らかに良い
- 自宅マシンの消費電力は`sensors`のPPTで読める(素数探索8スレッド稼働時でCPU 32W、Tctl 92.1℃)。システム全体55〜70Wは推定なので、ワットチェッカーがあれば確定できる

## ビルド

- C側: CMake+Ninja、`build-<Debug|Release|RelWithDebInfo|MinSizeRel|Unspecified>/`でビルド(詳細は自動メモリ参照)
- Java側(`java/`submodule): Gradleマルチモジュール。GMPバインディングは`gmp-linux`(Linux向け、正しい)と`gmp-msys2`(Windows向け、`unsigned long`が32bitに化ける罠あり)の2系統が混在(実際にどちらが使われているかは上記「既知の罠」参照)。素数探索本体は`java/prime-search`モジュール(`./gradlew :prime-search:installDist`)
