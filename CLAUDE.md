# study リポジトリの現状(2026-08-13時点)

## 進行中のプロジェクト: 2,097,152bit RSA鍵向け素数探索

- 2つの偶数(`even-number-2097152bit-<UUID>.txt`)それぞれについて、素数を1つ見つけようとしている
  - `037c1901-916f-4ce8-9461-cba9e1f4851f`
  - `49d09838-e81d-470e-a6eb-7157ea24ac6c`
- 既知素数篩(2.7×10^11まで)で候補を約4%まで削り込み済み。DBは`candidates.sqlite3`(SQLite)
- 残った候補をGMPの`mpz_probab_prime_p`(BPSW)でMiller-Rabin判定中。1件あたり実測で約7〜15時間かかる、非常に重い探索
- **systemdのuserサービスとして無人稼働中**。`systemctl --user status prime-search.target 'prime-search@*'`で状態確認、`journalctl --user -u prime-search@037c1901.service`でログ確認
- コードを変更したら`./gradlew :foreign:installDist`してから`systemctl --user restart prime-search.target`しないと反映されない(Gradle経由の起動は`./gradlew --stop`の巻き添えで落ちる事故が起きたため廃止した)
- 詳しい経緯・ハマりどころは`Claude`の自動メモリ(`build_workflow.md`, `gmp_windows_long_gotcha.md`, `java_gmp_binding_mismatch.md`, `even_number_file_format_history.md`, `old_main_pc_broken.md`, `prime_search_systemd_deployment.md`)を参照

## ビルド

- C側: CMake+Ninja、`build-<Debug|Release|RelWithDebInfo|MinSizeRel|Unspecified>/`でビルド(詳細は自動メモリ参照)
- Java側(`java/`submodule): Gradleマルチモジュール。GMPバインディングは`gmp-linux`(Linux向け、正しい)と`gmp-msys2`(Windows向け、`unsigned long`が32bitに化ける罠あり)の2系統が混在
