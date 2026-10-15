#!/bin/bash
# eme-oaep / emsa-pss を OpenSSL と突き合わせる。
#   rsa_pad_interop.sh <eme-oaep> <emsa-pss> <openssl>
#
# RSA演算そのものは両ツールに無いので、生のRSA演算 (rsa_padding_mode:none) を OpenSSL に任せて、
# パディングだけを次の2方向で確かめる。
#   A: ツールで encode -> OpenSSL の生RSA -> OpenSSL のOAEP復号 / PSS検証
#   B: OpenSSL のOAEP暗号化 / PSS署名 -> OpenSSL の生RSA -> ツールで decode / verify
# あわせて、パラメータ (ハッシュ、MGF1ハッシュ、ラベル、salt長) が食い違うと失敗すること、
# 8の倍数でない鍵長でもEMSA-PSSが自分で往復できること (verifyのemBitsの扱いの回帰) を見る。
set -u
OAEP=$1
PSS=$2
OSSL=$3

T=$(mktemp -d) || exit 1
trap 'rm -rf "$T"' EXIT
cd "$T" || exit 1

fail=0
pass=0
ok() { pass=$((pass + 1)); }
ng() { fail=$((fail + 1)); echo "NG: $*"; }

"$OSSL" genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out priv.pem 2>/dev/null || { echo "鍵の生成に失敗"; exit 1; }
"$OSSL" pkey -in priv.pem -pubout -out pub.pem || exit 1
"$OSSL" rsa -in priv.pem -RSAPublicKey_out -out pub1.pem 2>/dev/null   # PKCS#1 RSAPublicKey
"$OSSL" pkey -in priv.pem -outform DER -out priv.der
printf 'hello EME-OAEP / EMSA-PSS \x00\x01 binary' > msg.bin

hex() { printf %s "$1" | od -An -tx1 | tr -d ' \n'; }

# ---- EME-OAEP ----
# 形式: <oaep-hash> <mgf1-hash> <label>
for combo in "sha256 sha256 " "sha1 sha1 " "sha384 sha256 lbl" "sha256 sha1 ラベル" "sha512 sha512 x" "sha3-256 sha3-256 "; do
    set -- $combo
    H=$1; M=$2; L=${3:-}
    name="oaep $H/$M/label='$L'"
    OPT="-pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:$H -pkeyopt rsa_mgf1_md:$M -pkeyopt rsa_oaep_label:$(hex "$L")"

    # A
    if "$OAEP" encode msg.bin em.bin --key pub.pem --oaep-hash "$H" --mgf1-hash "$M" --label "$L" 2>/dev/null &&
       "$OSSL" pkeyutl -encrypt -pubin -inkey pub.pem -pkeyopt rsa_padding_mode:none -in em.bin -out ct.bin 2>/dev/null &&
       "$OSSL" pkeyutl -decrypt -inkey priv.pem $OPT -in ct.bin -out dec.bin 2>/dev/null &&
       cmp -s dec.bin msg.bin; then ok; else ng "$name A (tool encode -> openssl decrypt)"; fi

    # B
    if "$OSSL" pkeyutl -encrypt -pubin -inkey pub.pem $OPT -in msg.bin -out ct2.bin 2>/dev/null &&
       "$OSSL" pkeyutl -decrypt -inkey priv.pem -pkeyopt rsa_padding_mode:none -in ct2.bin -out em2.bin 2>/dev/null &&
       "$OAEP" decode em2.bin out.bin --key priv.der --oaep-hash "$H" --mgf1-hash "$M" --label "$L" 2>/dev/null &&
       cmp -s out.bin msg.bin; then ok; else ng "$name B (openssl encrypt -> tool decode)"; fi

    # 食い違うと失敗する (終了コード2)。ラベルが空でない組み合わせでは、空ラベルでも失敗するはず
    if [ -n "$L" ]; then
        "$OAEP" decode em2.bin out.bin --key priv.der --oaep-hash "$H" --mgf1-hash "$M" 2>/dev/null
        [ $? -eq 2 ] && ok || ng "$name ラベル不一致で失敗しない"
    fi
done
# ハッシュ・MGF1の不一致
"$OAEP" encode msg.bin em.bin --bits 2048 --oaep-hash sha256 --mgf1-hash sha256 2>/dev/null
"$OAEP" decode em.bin out.bin --bits 2048 --oaep-hash sha1 --mgf1-hash sha256 2>/dev/null; [ $? -eq 2 ] && ok || ng "oaep: oaep-hash不一致で失敗しない"
"$OAEP" decode em.bin out.bin --bits 2048 --oaep-hash sha256 --mgf1-hash sha1 2>/dev/null; [ $? -eq 2 ] && ok || ng "oaep: mgf1-hash不一致で失敗しない"
# 旧名 --hash は --oaep-hash の別名
"$OAEP" decode em.bin out.bin --bits 2048 --hash sha256 2>/dev/null && cmp -s out.bin msg.bin && ok || ng "oaep: --hash 別名"
# メッセージ長の境界 (2048bit, sha256: 最大190バイト)
head -c 190 /dev/urandom > m190; head -c 191 /dev/urandom > m191
"$OAEP" encode m190 e190 --bits 2048 2>/dev/null && "$OAEP" decode e190 d190 --bits 2048 2>/dev/null && cmp -s m190 d190 && ok || ng "oaep: 190バイトの往復"
"$OAEP" encode m191 e191 --bits 2048 2>/dev/null && ng "oaep: 191バイトが通ってしまう" || ok
# rsa-calc の出力のように先頭の 0x00 が落ちていても decode できる
"$OAEP" encode msg.bin em.bin --bits 2048 2>/dev/null
tail -c 255 em.bin | "$OAEP" decode - out.bin --bits 2048 2>/dev/null && cmp -s out.bin msg.bin && ok || ng "oaep: 先頭0x00が落ちたEM"

# ---- EMSA-PSS ----
# 形式: <pss-hash> <mgf1-hash> <saltlen>
for combo in "sha256 sha256 digest" "sha256 sha256 0" "sha256 sha256 max" "sha1 sha1 20" "sha384 sha256 digest" "sha256 sha1 32" "sha512 sha512 max" "sha3-256 sha3-256 digest"; do
    set -- $combo
    H=$1; M=$2; S=$3
    name="pss $H/$M/salt=$S"
    "$OSSL" dgst -"$H" -binary msg.bin > h.bin
    OPT="-pkeyopt rsa_padding_mode:pss -pkeyopt rsa_pss_saltlen:$S -pkeyopt digest:$H -pkeyopt rsa_mgf1_md:$M"

    # A (ハッシュ済みダイジェストを --prehashed で直接渡す)
    if "$PSS" encode h.bin em.bin --prehashed --key pub.pem --pss-hash "$H" --mgf1-hash "$M" --saltlen "$S" 2>/dev/null &&
       "$OSSL" pkeyutl -decrypt -inkey priv.pem -pkeyopt rsa_padding_mode:none -in em.bin -out sig.bin 2>/dev/null &&
       "$OSSL" pkeyutl -verify -pubin -inkey pub.pem $OPT -in h.bin -sigfile sig.bin >/dev/null 2>&1; then ok
    else ng "$name A (tool encode -> openssl verify)"; fi

    # B
    if "$OSSL" pkeyutl -sign -inkey priv.pem $OPT -in h.bin -out sig2.bin 2>/dev/null &&
       "$OSSL" pkeyutl -verifyrecover -pubin -inkey pub.pem -pkeyopt rsa_padding_mode:none -in sig2.bin -out em2.bin 2>/dev/null &&
       "$PSS" verify h.bin em2.bin --prehashed --key pub.pem --pss-hash "$H" --mgf1-hash "$M" --saltlen "$S" 2>/dev/null; then ok
    else ng "$name B (openssl sign -> tool verify)"; fi
done
# 食い違うと失敗する (終了コード2)
"$PSS" encode msg.bin em.bin --bits 2048 --saltlen 20 2>/dev/null
"$PSS" verify msg.bin em.bin --bits 2048 --saltlen 20 2>/dev/null; [ $? -eq 0 ] && ok || ng "pss: 正しいパラメータで失敗"
"$PSS" verify msg.bin em.bin --bits 2048 --saltlen auto 2>/dev/null; [ $? -eq 0 ] && ok || ng "pss: saltlen auto"
"$PSS" verify msg.bin em.bin --bits 2048 2>/dev/null; [ $? -eq 2 ] && ok || ng "pss: saltlen不一致で失敗しない"
"$PSS" verify msg.bin em.bin --bits 2048 --saltlen auto --pss-hash sha1 2>/dev/null; [ $? -eq 2 ] && ok || ng "pss: pss-hash不一致で失敗しない"
"$PSS" verify msg.bin em.bin --bits 2048 --saltlen auto --mgf1-hash sha1 2>/dev/null; [ $? -eq 2 ] && ok || ng "pss: mgf1-hash不一致で失敗しない"
printf 'x' | "$PSS" verify - em.bin --bits 2048 --saltlen auto 2>/dev/null; [ $? -eq 2 ] && ok || ng "pss: メッセージ改ざんを検出しない"
# 旧名 --hash は --pss-hash の別名
"$PSS" verify msg.bin em.bin --bits 2048 --saltlen 20 --hash sha256 2>/dev/null && ok || ng "pss: --hash 別名"
# 先頭の 0x00 が落ちたEM (rsa-calc の出力)
n=0
for i in $(seq 1 3000); do
    "$PSS" encode msg.bin e2.bin --bits 2048 2>/dev/null
    if [ "$(head -c1 e2.bin | od -An -tx1 | tr -d ' ')" = "00" ]; then
        tail -c +2 e2.bin | "$PSS" verify msg.bin - --bits 2048 2>/dev/null && ok || ng "pss: 先頭0x00が落ちたEM"
        n=1; break
    fi
done
[ $n -eq 1 ] || echo "(注意: 先頭0x00のEMが出なかったのでこの項目は未実施)"
# 鍵長が8の倍数でないときの往復 (verifyがemBitsを誤っていた不具合の回帰)。鍵が無くても --bits で試せる
for bits in 1025 1026 1027 1028 1029 1030 1031 1032 2049 2050 2051 2055 2056; do
    good=0
    for i in 1 2 3 4 5 6 7 8; do
        "$PSS" encode msg.bin e3.bin --bits $bits 2>/dev/null && "$PSS" verify msg.bin e3.bin --bits $bits 2>/dev/null && good=$((good + 1))
    done
    [ $good -eq 8 ] && ok || ng "pss: --bits $bits の往復 ($good/8)"
done
# 鍵ファイル形式 (PEM/DER、SPKI/PKCS#1、公開鍵/秘密鍵) から同じビット長が読める
for k in pub.pem pub1.pem priv.pem priv.der; do
    "$PSS" encode msg.bin /dev/null --key "$k" 2>&1 | grep -q "2048 bits" && ok || ng "pss: --key $k"
    "$OAEP" encode msg.bin /dev/null --key "$k" 2>&1 | grep -q "2048 bits" && ok || ng "oaep: --key $k"
done
# 不正な指定は終了コード1
"$PSS" encode msg.bin e --bits 2048 --salt-len 5 >/dev/null 2>&1; [ $? -eq 1 ] && ok || ng "pss: 不明なオプションがエラーにならない"
"$OAEP" encode msg.bin e --bits 2048 --key pub.pem >/dev/null 2>&1; [ $? -eq 1 ] && ok || ng "oaep: --bits と --key の併用がエラーにならない"

echo "rsa_pad_interop: ${pass} 件成功, ${fail} 件失敗"
[ $fail -eq 0 ]
