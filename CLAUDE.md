# study リポジトリの現状(2026-08-17時点)

`study`はREADME.mdが自称する通り雑多な個人リポジトリで、暗号プリミティブの未完成実装・i18nテスト・Minecraft関連ツールなど無関係な内容を多数含む。**現在アクティブに開発しているのは以下の2つ(素数探索とbitmessageアドレス探索)**で、それ以外の大半は過去の実験や中断した作業。ただし「古い＝再利用不可」とは限らない。実装済みの暗号プリミティブやユーティリティが今後別タスクの土台として流用できる可能性はあるので、無関係と決めつけず、既存コードに使えるものがないか一応grep等で確認してから書き始めるとよい。確認範囲はこのリポジトリ本体だけでなく、gitサブモジュール(`libstudy`, `java`, `math-scripts`, `python`)配下も対象に含めること。サブモジュールは`git clone`直後は中身が空なので、`git submodule update --init --recursive`していないと存在に気づけない点に注意。

## 進行中のプロジェクト: 2,097,152bit RSA鍵向け素数探索

- 2つの偶数(`even-number-2097152bit-<UUID>.txt`)それぞれについて、素数を1つ見つけようとしている
  - `037c1901-916f-4ce8-9461-cba9e1f4851f`
  - `49d09838-e81d-470e-a6eb-7157ea24ac6c`
- 既知素数篩(2.7×10^11まで)で候補を約4%まで削り込み済み。**DBは自宅のPostgres**(`jdbc:postgresql://127.0.0.1:5432/primesearch`、接続情報は`java/run-prime-search.sh`にある)。リポジトリ直下の`candidates.sqlite3`は移行前の遺物で、現在は使われていない
- 進捗(2026-10-03時点): 候補572,165件のうち**判定済み1,238件、素数はまだ0件**(2026-09-20時点は911件・1日約20件)。2台合計で1日約106件(`mr2fs`経路の実測。内訳は下記)。2つの偶数は`candidates`テーブルの`id`列で区別する(`id=251103173751557352`が037c1901、`id=5318918530104379150`が49d09838)
- **1本の素数を見つけるのに期待値で約31,000件**の判定が要る計算(2Mbit奇数が素数である確率2/ln(N)=1/726,818を、篩の残存率4.3%で割ったもの)。1日20件なので**1台だと1本あたり約4.3年、RSAに必要な2本で8〜9年**。2026-10-01から2台で1本ずつ並行して探している。**`mr2fs`経路(2026-10-03の実測)で計算し直すと、1本あたりの期待値は2台目(037c1901、1日約57.5件)が約1.48年、1台目(49d09838、1日約48.8件)が約1.74年。2本とも揃うまでは期待値で2年前後**(素数が出るまでの時間は指数分布に近くばらつきが大きい)。GMP単独だった頃は約2.5年と約3.2年(1台目でECMを並行すると約4.4年)だった。候補プール自体は各偶数に28.5万件あり、期待値では1つの偶数あたり9本前後の素数が埋まっているので枯渇の心配は無い
- 残った候補をMiller-Rabin判定中。GMP単独(`mpz_probab_prime_p`のBPSW)だと1件あたり実測で約7〜15時間かかる、非常に重い探索。**2026-10-02から、底2の判定はFLINT `fft_small`経由の`mr2fs`で行い、GMP単独の約1.7〜1.85倍速い**(本番8スレッドの実測。1スレッドの単体測定では約2.1倍)(下の「mr2fs高速経路」)
- **systemdのuserサービスとして、2台で無人稼働中**(2026-10-01〜)。各マシンで`systemctl --user status prime-search.target 'prime-search@*'`を実行すると状態を確認できる。ログは`journalctl --user -u prime-search@<UUID接頭辞>.service`で見る
  - **このマシン(1台目)は`49d09838`専任(8スレッド)。** ここの`prime-search@037c1901.service`は意図的にmaskedにしてある。1台で2本同時に動かすのは無理だった。maskedを見て「止まっている、直さなきゃ」と勝手に起動しないこと
  - **`037c1901`は2台目のミニPCが専任で担当(8スレッド)。** ホスト名とアドレスは`CLAUDE.local.md`に書いてある。2台目にはリポジトリを置いていない。`~/prime-search/`に、installDistの成果物(`install/`)、偶数ファイル、2台目専用の`run-prime-search.sh`を置いている。DBへは1台目のPostgresにLAN経由で直接つなぐ(`sslmode=require`)。Tailscaleは使っていない
  - 2台の分担は偶数単位で分けている(同じ偶数を2台で共有しない)。共有しても候補のclaimで二重着手は防げる。ただし素数が見つかったかどうかの判定が起動時にしか無いので、片方が見つけても、もう片方は手動で止めるまで計算を続けてしまう
- コードを変更したら`./gradlew :prime-search:installDist`してから`systemctl --user restart prime-search.target`しないと反映されない(Gradle経由の起動は`./gradlew --stop`の巻き添えで落ちる事故が起きたため廃止した)。**2台目にも、`build/install/prime-search/`を`~/prime-search/install/`へrsyncしてから再起動する必要がある**
- `rotate-db-password.sh`は、`WORKERS`に書いた全マシン(sshで操作)の`db.env`を書き換えて再起動する。マシンを増やしたら`WORKERS`に追記すること。**全台が同時にバッチ境界にいないと、どこかの判定中の8件が無駄になる。** 境界の時刻はマシンごとにずれていくので、`--wait`は数日かかることがある
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

**下地自体は完成しているので、気が変わればすぐ再開できる**: PostgresはTailscale経由で外部から待ち受けており接続確認済み(アドレスは`CLAUDE.local.md`)、`PrimeSearchTask2`にアトミックなclaimと`--stale-hours`があるのでスポット回収にも耐える。gcloudは`~/google-cloud-sdk`に導入済み。

- 進捗管理コードを`java/foreign`・`java/develop`から専用モジュール`java/prime-search`(`com.github.teruteru.primesearch`)へ切り出し済み(java-studyコミット`00366cc2`)。**`PrimeSearch`/`PrimeSearchTask2`/`Result`/`Gmp`facadeは`foreign`ではなく`prime-search`が正**
- DB接続は`SQLiteDataSource`決め打ちから`DriverManager.getConnection(DB_URL)`に統一済み。`DB_URL`のスキーム(`jdbc:sqlite:`/`jdbc:postgresql:`)で自動的にドライバが切り替わる
- 複数マシンの二重着手を防ぐため、候補行のアトミックなclaim(`--stale-hours`オプション、既定24h)を`PrimeSearchTask2`に実装済み
- **完了済み**(2026-09-20に実機で確認): 自宅Postgresサーバー導入、`candidates.sqlite3`からのデータ移行(572,165件がPostgresに入っている)、`run-prime-search.sh`のPostgres/新launcherへの切り替え、Tailscale接続設定(このマシンが参加済み。ノード名とアドレスは`CLAUDE.local.md`)
- **未着手**: GCEインスタンスの実際の構築。`gcloud`コマンドすら入っていない。Tailscaleに参加しているノードもこのマシン1台だけ
- **GMPの内部を速くする余地はほとんど無い(これは今も正しい)。** 1件9.5時間という速度は、GMPのSSA-FFTを使う限りほぼ限界(2Mbitの冪剰余は約200万回の2Mbit二乗算)。**ただし乗算そのものをFLINT `fft_small`に替えれば約2倍になる。** 2026-10-02に確認した(下の「mr2fs高速経路」)。「理論上10時間前後」という見積もりはGMPのFFT前提だった
- **SMTはほとんど効かない(実測)。** 2Mbitの`mpn_sqr`は物理コア数までは劣化ゼロでスケールするが、そこを超えると1.68倍遅くなる。8スレッドの実効は4.76コア相当で、4スレッドに対して19%しか増えない。**vCPU数を性能と読んではいけない**(`c3-standard-88`の88vCPUは約52コア相当)
- 詳細は自動メモリ`gce_postgres_scaleout_plan.md`

### mr2fs高速経路(2026-10-02〜)

2,097,152bitの底2のMiller-Rabinを、GMPの`mpn_sqr`+`mpn_redc_n`の代わりに、FLINT 3.4.0の`fft_small`(`mpn_ctx_mpn_mul`)とMontgomery法で行うC実装(`src/mr2fs.c`)。Javaからは`java/prime-search`の`Mr2fs`クラスがFFMで呼ぶ。

- **効果(実測)**: 1ステップ(二乗+剰余)は、GMPの約10.2msに対して、1スレッドで5.12ms、8スレッド同時で5.84ms。1件あたりでは、本番サイズ(32,768リム、指数全ビット)で`mr2fs` 11,031秒対GMP 23,021秒(約2.1倍)。**残余はmpz_powmとビット単位で一致を確認済み**(`mr2fs_full`)。**本番(8スレッド、2026-10-03)の実測**: 1台目 1件3.94時間(16件、3.79〜4.10)でGMP単独の7.30時間の1.85倍、2台目 1件3.34時間(24件、3.24〜3.37)で5.77時間の1.73倍。1台目は熱で頭打ち(Tctl 92℃)のため、単体測定(3.06時間)より約30%遅い
- **使い方**: `PrimeSearchTask2`は、`Mr2fs.strongBase2`が0(合成数)ならそこで確定し、通った候補(約1/3万)と使えない場合だけGMPのBPSWに任せる。素数の確認は常にGMPなので、結果の意味は変わらない。ライブラリは`-Dcom.github.teruteru.mr2fs.library=<.soの絶対パス>`で渡す(`java/run-prime-search.sh`が`build-Release/src/libmr2fs.so.1.0.0`を渡す)。見つからなければGMP単独に自動で落ちるので壊れはしない。**起動ログの「mr2fs高速経路: 有効」を必ず確認すること**
- **FLINTは必ず`./configure --enable-avx2`付きでビルドする。** 付け忘れると`fft_small`がAVX2を使わず約3割遅い(動作と結果は同じなので気づきにくい)。`/usr/local/flint-3.4.0`が現在AVX2版(`cm-0.4.4`もこれを使うので、入れ替えるときは`cm`の動作も確認すること)。`--enable-avx512`は無効(3.4.0の`fft_small`にAVX-512の経路が無く、速度は同じ)
- **`MALLOC_TOP_PAD_=268435456`をsystemdのユニット(`~/.config/systemd/user/prime-search@.service`の`[Service]`、リポジトリ外)に設定済み。** GMPのFFTが1ステップごとに大きな一時領域をmalloc/freeし、glibcがそのたびにヒープを縮めてページフォルトを起こしていた。GMP経路で約7〜8%速くなる(1スレッド10.23→9.53ms、8スレッド11.05→10.16ms)。`MALLOC_MMAP_THRESHOLD_`や`MALLOC_TRIM_THRESHOLD_`を**単独で**設定すると、glibcの動的調整が切れてかえって約10%遅くなるので、`TOP_PAD`だけにすること。`mr2fs`経路には効かない(`fft_small`が内部バッファを使い回すため)。2台目にも同じ設定が要る
- **2台目への展開は、タスクの切れ目に手動で行う**: `libmr2fs.so`、`/usr/local/flint-3.4.0`(AVX2版)、2台目の`run-prime-search.sh`への`JAVA_OPTS`の1行、ユニットへの`Environment=`
- **スレッドごとに`mpn_ctx`を持つ**(RSSは1スレッド約11MB、8スレッドで約58MB)。長時間の連続実行で劣化しないことは、1件ぶんの突き合わせで確認済み
- 次の伸びしろ(未着手): 剰余の`q = lo(T)*ip`が下位半分しか要らないのに全長乗算をしている点と、定数側(`ip`、`m`)のFFT変換の再利用。`fft_small`の内部関数を直接呼ぶ必要がある
- 詳細は`src/mr2fs.c`冒頭のコメントと、`src/mr2fs_test.c`、`src/mr2fs_full.c`

## 進行中のプロジェクト: bitmessageアドレス探索(2026-09-19〜)

先頭に多くのゼロを持つripeのbitmessageアドレスを、既存の公開鍵ファイル群から総当たりで探す。
外付けHDD(ラベル`HD-NRLD-A`)の`避難所/keys/public/publicKeys{0..255}.bin`(各1,090,519,040バイト = 16,777,216鍵 × 65バイト)を使う。HDDのマウント先は環境によって変わる(udisks2の更新で`/media`配下から`/run/media`配下に移ったことがある)ので、パスを決め打ちせず`findmnt -rno TARGET -S LABEL=HD-NRLD-A`で調べること。

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
- **Ubuntu 26.04(GCC 15)ではGMP 6.3.0のconfigureが通らない**: GCC 15は既定の言語規格がC23になった。C23では`void g(){}`が引数なしを意味するので、configureの`long long reliability test`がコンパイルエラーになる。その結果、`could not find a working compiler`で止まる。`./configure CFLAGS="-march=native -O3 -std=gnu17"`とすれば通る。m4も必要
- **GMPの`gmp-mparam.h`を`tuneup`の結果に差し替えたら、`make clean`してからビルドし直す**: GMPのMakefileはこのヘッダへの依存関係を追跡していない。ただ`make`しても何も再コンパイルされず、計測した閾値がライブラリに入らない

## 計算資源

- 1台目(このマシン): Ryzen 7 8745H(Zen 4、8コア16スレッド)、RAM 27GB程度(「ミニコンピュータ」)。Miller-Rabin判定を15スレッド並列で回すとメモリ帯域が奪い合いになり実効速度が半減する現象を確認済み
  - 素数探索8スレッドだけを動かしているとき: 1件約7.3時間、1日約26件(2026-09-14〜18の実績)。全コア約3.6GHzで、Tctl 92℃・PPT 34Wに張り付いている。冷却か電力上限で頭打ちになっているとみられる
  - **`mr2fs`経路(2026-10-02 22:45〜、ECM停止中)**: 1件3.94時間、1日約48.8件(2026-10-03実測)。1件あたりのCPU電力量は約18Wh(PPT平均約37W)で、GMP単独時の約31Whから約4割減。熱で頭打ちなのは変わらない
  - **ECMなど別の重い計算を並行させると遅くなる。** 2026-09-27以降はECM(`run-ecm-1148-t50.sh`など、4プロセス)を並行させていて、1件約9.9時間、1日約19件に落ちている。素数探索の処理量の約26%をECMに譲っている計算になる **2026-10-02からECMは停止中で、素数探索の2本(037c1901と49d09838)が見つかるまで再開しない方針(ユーザー決定)。** 再開を提案する前にこの方針を確認すること。自動再開の仕組み(cron/タイマー)は無い
- 2台目(2026-10-01導入のミニPC): Ryzen 9 PRO 8945HS(Zen 4、8コア16スレッド)、RAM 12GB、Ubuntu 26.04を新規インストール。GMPはこのマシン上で`-march=native`と`tuneup`を使って自家ビルドし、`/usr/local/lib`に入れてある。有線LANは未接続で、Wi-Fiでつながっている
  - 素数探索8スレッドで1件5.77時間、1日約33件。全コア約4.53GHz、PPT 54W、Tctl 87℃(2026-10-02の実測)。1件あたりのCPU電力量は約39Whで、1台目(単独時の約31Wh、ECM並行時の約42Wh)と大差ない
  - **`mr2fs`経路(2026-10-02 22:53〜)**: 1件3.34時間、1日約57.5件(2026-10-03実測)。1件あたりのCPU電力量は約22.5Wh(PPT平均約54W)で、GMP単独時の約39Whから約4割減
  - 1台目との速さの差は、ほぼクロックの差(冷却・電力上限の違い)による。2026-10-02に、1台目でビルドしたGMP(GCC 13)と2台目でビルドしたGMP(GCC 15)を両方のマシンで入れ替えて測ったが、ライブラリによる差は無かった
- 故障中の旧メインPC(RAM 128GB)が自宅にある(型番未確認)。**ただし電源に余裕が無いため、計算機の追加は行わない方針(下記)。** 修理しても投入しない
- **電源の制約(2026-10-02): 2台目を入れた時点で、タコ足配線気味になり電源周りが逼迫した。これ以上マシンを増やさない。** したがって速度を上げる手段は、今ある2台の中で完結するもの(ソフトウェアの高速化、1台目の熱によるクロック低下の解消、ECMなど他の重い計算との住み分け)に限る。消費電力が増える案(PPT上限を上げる、マシンの追加、旧メインPCの修理)は、提案する前に必ずこの制約を確認すること
- **GCE投入は費用が見合わず見送り**(上記参照)。`java/postgres-db-migration-TODO.txt`は初期の移行メモで、移行自体は完了済みなので歴史的資料
- ~~現実的に効くのは旧メインPCの修理~~: **電源の制約により取り下げ**(上記)。2026-10-02に`mr2fs`でソフトウェア側が約2倍になったので、今は台数を増やさなくても進められる
- 自宅マシンの消費電力は`sensors`のPPTで読める(素数探索8スレッド稼働時でCPU 32W、Tctl 92.1℃)。システム全体55〜70Wは推定なので、ワットチェッカーがあれば確定できる

## ビルド

- C側: CMake+Ninja、`build-<Debug|Release|RelWithDebInfo|MinSizeRel|Unspecified>/`でビルド(詳細は自動メモリ参照)
- Java側(`java/`submodule): Gradleマルチモジュール。GMPバインディングは`gmp-linux`(Linux向け、正しい)と`gmp-msys2`(Windows向け、`unsigned long`が32bitに化ける罠あり)の2系統が混在(実際にどちらが使われているかは上記「既知の罠」参照)。素数探索本体は`java/prime-search`モジュール(`./gradlew :prime-search:installDist`)
