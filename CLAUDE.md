# study リポジトリの現状(2026-08-13時点)

`study`はREADME.mdが自称する通り雑多な個人リポジトリで、暗号プリミティブの未完成実装・i18nテスト・Minecraft関連ツールなど無関係な内容を多数含む。**現在アクティブに開発しているのは以下の素数探索プロジェクトのみ**で、それ以外の大半は過去の実験や中断した作業。ただし「古い＝再利用不可」とは限らない。実装済みの暗号プリミティブやユーティリティが今後別タスクの土台として流用できる可能性はあるので、無関係と決めつけず、既存コードに使えるものがないか一応grep等で確認してから書き始めるとよい。確認範囲はこのリポジトリ本体だけでなく、gitサブモジュール(`libstudy`, `java`, `math-scripts`, `python`)配下も対象に含めること。サブモジュールは`git clone`直後は中身が空なので、`git submodule update --init --recursive`していないと存在に気づけない点に注意。

## 進行中のプロジェクト: 2,097,152bit RSA鍵向け素数探索

- 2つの偶数(`even-number-2097152bit-<UUID>.txt`)それぞれについて、素数を1つ見つけようとしている
  - `037c1901-916f-4ce8-9461-cba9e1f4851f`
  - `49d09838-e81d-470e-a6eb-7157ea24ac6c`
- 既知素数篩(2.7×10^11まで)で候補を約4%まで削り込み済み。DBは`candidates.sqlite3`(SQLite)
- 残った候補をGMPの`mpz_probab_prime_p`(BPSW)でMiller-Rabin判定中。1件あたり実測で約7〜15時間かかる、非常に重い探索
- **systemdのuserサービスとして無人稼働中**。`systemctl --user status prime-search.target 'prime-search@*'`で状態確認、`journalctl --user -u prime-search@037c1901.service`でログ確認
- コードを変更したら`./gradlew :foreign:installDist`してから`systemctl --user restart prime-search.target`しないと反映されない(Gradle経由の起動は`./gradlew --stop`の巻き添えで落ちる事故が起きたため廃止した)
- 詳しい経緯・ハマりどころは`Claude`の自動メモリ(`build_workflow.md`, `gmp_windows_long_gotcha.md`, `java_gmp_binding_mismatch.md`, `even_number_file_format_history.md`, `old_main_pc_broken.md`, `prime_search_systemd_deployment.md`)を参照

### 既知の罠

- **GMPバインディング混在**: Javaの実行系コード(`PrimeSearch.java`ほとんど全部)は`gmp.linux`ではなく`gmp.msys2`パッケージをimportしている。`gmp-msys2`は`unsigned long`引数をWindows(LLP64)に合わせて32bitの`int`として扱うため、`mpz_add_ui`/`mpz_set_ui`/`mpz_fdiv_ui`等に2^31を超える値を渡すとLinux上でもサイレントに壊れる。`gmp.linux`(64bit、正しい実装)を使っているのは`GMPWrapper.java`のみ。GMP呼び出しを含むJavaコードを触るときは、どちらのパッケージをimportしているか必ず確認する
- **even-numberファイルの10進/16進混在**: 旧1,048,576bit世代(外付けHDD保管)は10進数、現行2,097,152bit世代は16進数。10進ファイルを誤って16進として読んでも構文エラーにならず無音で違う値になる(逆方向は`a`〜`f`混入で即エラーになるため気づきやすい)。C側`load_even_base`とJava側`PrimeSearch`には、a-fを1つも含まないファイルを警告するチェックを追加済み

## 計算資源

- 現在稼働中のマシンはRAM 27GB程度(「ミニコンピュータ」)。Miller-Rabin判定を15スレッド並列で回すとメモリ帯域が奪い合いになり実効速度が半減する現象を確認済み
- 故障中の旧メインPC(RAM 128GB)が自宅にあり、修理すれば計算資源として追加投入できる(型番未確認、修理店に持ち込み待ち)
- `java/postgres-db-migration-TODO.txt`に、候補DBを将来SQLiteからPostgresへ移行してマルチマシン構成にする案を記録済み(旧メインPCが復旧したら着手予定)

## ビルド

- C側: CMake+Ninja、`build-<Debug|Release|RelWithDebInfo|MinSizeRel|Unspecified>/`でビルド(詳細は自動メモリ参照)
- Java側(`java/`submodule): Gradleマルチモジュール。GMPバインディングは`gmp-linux`(Linux向け、正しい)と`gmp-msys2`(Windows向け、`unsigned long`が32bitに化ける罠あり)の2系統が混在(実際にどちらが使われているかは上記「既知の罠」参照)
