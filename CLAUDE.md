# study リポジトリの現状(2026-08-17時点)

`study`はREADME.mdが自称する通り雑多な個人リポジトリで、暗号プリミティブの未完成実装・i18nテスト・Minecraft関連ツールなど無関係な内容を多数含む。**現在アクティブに開発しているのは以下の素数探索プロジェクトのみ**で、それ以外の大半は過去の実験や中断した作業。ただし「古い＝再利用不可」とは限らない。実装済みの暗号プリミティブやユーティリティが今後別タスクの土台として流用できる可能性はあるので、無関係と決めつけず、既存コードに使えるものがないか一応grep等で確認してから書き始めるとよい。確認範囲はこのリポジトリ本体だけでなく、gitサブモジュール(`libstudy`, `java`, `math-scripts`, `python`)配下も対象に含めること。サブモジュールは`git clone`直後は中身が空なので、`git submodule update --init --recursive`していないと存在に気づけない点に注意。

## 進行中のプロジェクト: 2,097,152bit RSA鍵向け素数探索

- 2つの偶数(`even-number-2097152bit-<UUID>.txt`)それぞれについて、素数を1つ見つけようとしている
  - `037c1901-916f-4ce8-9461-cba9e1f4851f`
  - `49d09838-e81d-470e-a6eb-7157ea24ac6c`
- 既知素数篩(2.7×10^11まで)で候補を約4%まで削り込み済み。DBは`candidates.sqlite3`(SQLite)
- 残った候補をGMPの`mpz_probab_prime_p`(BPSW)でMiller-Rabin判定中。1件あたり実測で約7〜15時間かかる、非常に重い探索
- **systemdのuserサービスとして無人稼働中**。`systemctl --user status prime-search.target 'prime-search@*'`で状態確認、`journalctl --user -u prime-search@037c1901.service`でログ確認
- コードを変更したら`./gradlew :prime-search:installDist`してから`systemctl --user restart prime-search.target`しないと反映されない(Gradle経由の起動は`./gradlew --stop`の巻き添えで落ちる事故が起きたため廃止した)
- 詳しい経緯・ハマりどころは`Claude`の自動メモリ(`build_workflow.md`, `gmp_windows_long_gotcha.md`, `java_gmp_binding_mismatch.md`, `even_number_file_format_history.md`, `old_main_pc_broken.md`, `prime_search_systemd_deployment.md`, `gce_postgres_scaleout_plan.md`, `verify_portability_claims_rigorously.md`)を参照

### GCEスポットインスタンス+Postgresへのスケールアウト計画(2026-08-17〜、進行中)

自宅マシンだけで回し続けると電気代がかさむため、GCEのスポットインスタンス(c3-standard-88等)をワーカーとして追加投入する計画が進行中。DBは自宅にPostgresを置き、TailscaleでGCEと接続する構成。

- 進捗管理コードを`java/foreign`・`java/develop`から専用モジュール`java/prime-search`(`com.github.teruteru.primesearch`)へ切り出し済み(java-studyコミット`00366cc2`)。**`PrimeSearch`/`PrimeSearchTask2`/`Result`/`Gmp`facadeは`foreign`ではなく`prime-search`が正**
- DB接続は`SQLiteDataSource`決め打ちから`DriverManager.getConnection(DB_URL)`に統一済み。`DB_URL`のスキーム(`jdbc:sqlite:`/`jdbc:postgresql:`)で自動的にドライバが切り替わる
- 複数マシンの二重着手を防ぐため、候補行のアトミックなclaim(`--stale-hours`オプション、既定24h)を`PrimeSearchTask2`に実装済み
- **未着手**: 自宅Postgresサーバー導入、Tailscale接続設定、`candidates.sqlite3`からのデータ移行、`run-prime-search.sh`のPostgres/新launcherへの切り替え(本番停止を伴うため要事前確認)、GCEインスタンスの実際の構築。詳細は自動メモリ`gce_postgres_scaleout_plan.md`

### 既知の罠

- **`./gradlew :foreign:installDist`を実行しないこと(切り替え未完了の間)**: `foreign`からは`prime-search`への切り出しに伴い`search`サブコマンドを削除済みだが、本番`run-prime-search.sh`はまだ`java/foreign/build/install/foreign/bin/foreign search`を指したまま(古いビルド成果物で動いている)。`run-prime-search.sh`を`prime-search`側に切り替えるまでの間に`:foreign:installDist`を実行すると、`search`の無い`foreign`で上書きされ本番サービスが壊れる
- **GMPバインディング混在**: `gmp-linux`(Linux向け、64bit、正しい)と`gmp-msys2`(Windows/LLP64向け、`unsigned long`が32bitの`int`として扱われる)の2系統が混在している。`gmp-msys2`を使うコードで`mpz_add_ui`/`mpz_set_ui`/`mpz_fdiv_ui`等に2^31を超える値を渡すとLinux上でもサイレントに壊れる。`prime-search`モジュールの`Gmp`facadeは`gmp-linux`を正しく使っているが、`foreign`に残っている一部の旧コード(`PrimeSearchTask.java`など、未使用の遺物)は今も`gmp-msys2`をimportしている。GMP呼び出しを含むJavaコードを触るときは、どちらのパッケージをimportしているか必ず確認する
- **even-numberファイルの10進/16進混在**: 旧1,048,576bit世代(外付けHDD保管)は10進数、現行2,097,152bit世代は16進数。10進ファイルを誤って16進として読んでも構文エラーにならず無音で違う値になる(逆方向は`a`〜`f`混入で即エラーになるため気づきやすい)。C側`load_even_base`とJava側`PrimeSearch`には、a-fを1つも含まないファイルを警告するチェックを追加済み

## 計算資源

- 現在稼働中のマシンはRAM 27GB程度(「ミニコンピュータ」)。Miller-Rabin判定を15スレッド並列で回すとメモリ帯域が奪い合いになり実効速度が半減する現象を確認済み
- 故障中の旧メインPC(RAM 128GB)が自宅にあり、修理すれば計算資源として追加投入できる(型番未確認、修理店に持ち込み待ち)
- GCEスポットインスタンスの追加投入を計画中(上記参照)。`java/postgres-db-migration-TODO.txt`は初期の移行メモ、現状は`gce_postgres_scaleout_plan.md`(自動メモリ)を参照

## ビルド

- C側: CMake+Ninja、`build-<Debug|Release|RelWithDebInfo|MinSizeRel|Unspecified>/`でビルド(詳細は自動メモリ参照)
- Java側(`java/`submodule): Gradleマルチモジュール。GMPバインディングは`gmp-linux`(Linux向け、正しい)と`gmp-msys2`(Windows向け、`unsigned long`が32bitに化ける罠あり)の2系統が混在(実際にどちらが使われているかは上記「既知の罠」参照)。素数探索本体は`java/prime-search`モジュール(`./gradlew :prime-search:installDist`)
