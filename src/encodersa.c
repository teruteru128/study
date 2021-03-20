
#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <uuid/uuid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  char *cwd = getcwd(NULL, 0);
  printf("cwd is %s\n", cwd);
  free(cwd);
  int ret = EXIT_FAILURE;
  // ファイルから素数を読み込む
  if (argc != 3)
  {
    printf("%s file1 file2\n", argv[0]);
    return EXIT_SUCCESS;
  }
  char *infile1 = argv[1];
  char *infile2 = argv[2];
  uuid_t uuid;
  char uuidstr[UUID_STR_LEN];
  uuid_generate_random(uuid);
  uuid_unparse_lower(uuid, uuidstr);
  BN_CTX *ctx = NULL;
  ctx = BN_CTX_new();
  if (ctx == NULL)
  {
    perror("BN_CTX_new");
    goto err;
  }
  BN_CTX_start(ctx);
  BIGNUM *r0 = NULL, *r1 = NULL, *r2 = NULL, *n = BN_new(), *e = BN_new(),
         *d = BN_secure_new(), *p = BN_secure_new(), *q = BN_secure_new(), *dmp = BN_secure_new(), *dmq = BN_secure_new(), *iqmp = BN_secure_new();
  if (iqmp == NULL)
  {
    perror("BN_new");
    goto err;
  }
  if (!BN_set_word(e, 65537))
  {
    perror("BN_set_word");
    goto err;
  }
  r0 = BN_CTX_get(ctx);
  r1 = BN_CTX_get(ctx);
  r2 = BN_CTX_get(ctx);
  if (r2 == NULL)
  {
    perror("BN_CTX_get");
  }

  {
    FILE *in = NULL;
    int size = 0;
    char buf[65537];
    char *trash;
    in = fopen(infile1, "r");
    if (in == NULL)
    {
      perror("fopen");
      goto err;
    }
    trash = fgets(buf, 65537, in);
    fclose(in);
    if (BN_hex2bn(&p, buf) == 0)
    {
      perror("");
    }
    printf("p : %dbits\n", BN_num_bits(p));

    in = fopen(infile2, "r");
    if (in == NULL)
    {
      perror("fopen");
      goto err;
    }
    trash = fgets(buf, 65537, in);
    fclose(in);
    if (BN_hex2bn(&q, buf) == 0)
    {
      perror("");
    }
    printf("q : %dbits\n", BN_num_bits(q));
    memset(buf, 0, 65537);
  }
  if (BN_cmp(p, q) < 0)
  {
    printf(_("q is larger\n"));
    BN_swap(p, q);
  }
  else
  {
    printf(_("p is larger\n"));
  }
  BN_mul(n, p, q, ctx);
  // 計算
  if (!BN_sub(r1, p, BN_value_one()))
  {
    goto err;
  }
  /* q - 1 */
  if (!BN_sub(r2, q, BN_value_one()))
    goto err;
  /* (p - 1)(q - 1) */
  if (!BN_mul(r0, r1, r2, ctx))
    goto err;
  {
    BIGNUM *pr0 = BN_new();

    if (pr0 == NULL)
      goto err;

    BN_with_flags(pr0, r0, BN_FLG_CONSTTIME);
    if (!BN_mod_inverse(d, e, pr0, ctx))
    {
      perror(ERR_reason_error_string(ERR_get_error()));
      BN_free(pr0);
      goto err; /* d */
    }
    /* We MUST free pr0 before any further use of r0 */
    BN_free(pr0);
  }
  {
    BIGNUM *dtmp = BN_new();

    if (d == NULL)
      goto err;

    BN_with_flags(dtmp, d, BN_FLG_CONSTTIME);

    /* calculate d mod (p-1) and d mod (q - 1) */
    if (!BN_mod(dmp, dtmp, r1, ctx) || !BN_mod(dmq, dtmp, r2, ctx))
    {
      BN_free(dtmp);
      goto err;
    }

    /* We MUST free d before any further use of rsa->d */
    BN_free(dtmp);
  }

  {
    BIGNUM *ptmp = BN_new();

    if (p == NULL)
      goto err;
    BN_with_flags(ptmp, p, BN_FLG_CONSTTIME);

    /* calculate inverse of q mod p */
    if (!BN_mod_inverse(iqmp, q, ptmp, ctx))
    {
      BN_free(ptmp);
      goto err;
    }

    /* We MUST free p before any further use of rsa->p */
    BN_free(ptmp);
  }
  RSA *rsa = RSA_new();
  if (rsa == NULL)
  {
    perror("RSA_new");
    goto err;
  }
  printf("bit size: %d\n", BN_num_bits(n));
  RSA_set0_key(rsa, n, e, d);
  RSA_set0_factors(rsa, p, q);
  RSA_set0_crt_params(rsa, dmp, dmq, iqmp);
#if 1
  // 検証
  int err = RSA_check_key(rsa);
  printf("RSA_check_key is %d\n", err);
  if (err == 1)
  {
    printf("RSA is OK\n");
  }
  else if (err == 0)
  {
    printf("RSA is NG : %s\n", ERR_reason_error_string(ERR_get_error()));
    goto err;
  }
  else
  {
    perror(ERR_reason_error_string(ERR_get_error()));
    goto err;
  }
#endif
  // ファイル書き出し

  char outfile[PATH_MAX];
  snprintf(outfile, PATH_MAX, "%dbit-%s-priv.pem", BN_num_bits(n), uuidstr);
  BIO *bio = BIO_new_file(outfile, "w");
  if (bio == NULL)
  {
    perror(ERR_reason_error_string(ERR_get_error()));
    goto err;
  }
  if (!PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL))
  {
    perror(ERR_reason_error_string(ERR_get_error()));
  }
  BIO_flush(bio);
  BIO_free(bio);
  ret = EXIT_SUCCESS;
err:
  BN_CTX_end(ctx);
  BN_CTX_free(ctx);
#if 0
  BN_free(n);
  BN_free(e);
  BN_free(d);
  BN_free(p);
  BN_free(q);
  BN_free(dmp);
  BN_free(dmq);
  BN_free(iqmp);
#endif
  RSA_free(rsa);
  return ret;
}
