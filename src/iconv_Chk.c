#include <stdio.h>
#include <iconv.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* fgetsの指定バイト数 */
#define READ_SIZE 4096
/* 置き換え文字のサイズ */
#define CONTENT_SIZE 10
/*ダメ文字置き換え文字*/
#define DAMEMOJI  "?"

char outbuf[CONTENT_SIZE];

int convert(char const *src,
            char const *dest,
            char const *text,
            char *buf,
            size_t bufsize);

void chr_conv(char const *str,char *hairtetu[],int cnt);


/*
---------------------------------------------------------------
文字コード変換を行う
１文字ずつ文字コード変換を行う。変換不可文字があった場合は「?」に置き換える

【引数】
   1:utf8-->cp932
   2:cp932-->utf8

   ex)入力文字列をUTF8からcp932に変換して表示
    echo "変換文字"|./iconv_Chk 1
   
   ex)入力ファイルをUTF8からcp932に変換して表示
    cat aaa.txt|./iconv_Chk 1

【コンパイル】
gcc -o iconv_Chk iconv_Chk.c 

【UTF8ロケールで実行】
export LANG=ja_JP.utf8 && echo "日本語表示だよこれ〠と、この３つ㊽㊾㊿はｃｐ９３２では表示出来ないよ！"|./iconv_Chk 1

実行＆確認（結果をUFT8に変換して表示）
export LANG=ja_JP.utf8 && echo "日本語表示だよこれ〠と、この３つ㊽㊾㊿はｃｐ９３２では表示出来ないよ！"|./iconv_Chk 1|iconv -f cp932 -t utf8

【SJISロケールで実行】
export LANG=ja_JP.sjis && echo "日本語表示だよこれ〜と、この３つ｜＝￥はｃｐ９３２でも表示出来る！"|./iconv_Chk 2

【コンパイル＆実行】
gcc -o iconv_Chk iconv_Chk.c && export LANG=ja_JP.utf8 && echo "日本語表示aああbいcうdえ"|./iconv_Chk 1


------テストケース--------------
①UTF8→cp932変換
・データ[test_utf8.txt]
　「日本語表示だよこれ〠と、この３つ㊽㊾㊿はｃｐ９３２では表示出来ないよ！」
を文字コードUTF8で作成

・UTF8環境で実行
　UTF8→cp932変換 結果をiconvにパイプして変換不可文字「〠㊽㊾㊿」が「？」に変換されていること

　・環境設定
   export LANG=ja_JP.utf8
　・実行
　 cat test_utf8.txt|./iconv_Chk 1|iconv -f cp932 -t utf8

・SJIS環境で実行
　UTF8→cp932変換 結果をiconvにパイプして変換不可文字「〠㊽㊾㊿」が「？」に変換されていること

　・環境設定
   export LANG=ja_JP.sjis
　・実行
　 cat test_utf8.txt|./iconv_Chk 1|iconv -f cp932 -t utf8

②cp932→UTF8変換
・データ[test_sjis.txt]
　「日本語表示だよこれ〜と、この３つ｜＝￥はｃｐ９３２でも表示出来る！」
を文字コードcp932で作成

・UTF8環境で実行
　cp932→UTF8変換 全て変換されていること

　・環境設定
   export LANG=ja_JP.utf8
　・実行
　 cat test_sjis.txt|./iconv_Chk 2

・SJIS環境で実行
　cp932→UTF8変換 変換 全て変換されていること

　・環境設定
   export LANG=ja_JP.sjis
　・実行
　 cat test_sjis.txt|./iconv_Chk 2

テスト実行
echo "-------------------------------------------------------------"
export LANG=ja_JP.utf8
cat test_utf8.txt|./iconv_Chk 1|iconv -f cp932 -t utf8
echo "-------------------------------------------------------------"
export LANG=ja_JP.sjis
cat test_utf8.txt|./iconv_Chk 1|iconv -f cp932 -t utf8
echo "-------------------------------------------------------------"
export LANG=ja_JP.utf8
cat test_sjis.txt|./iconv_Chk 2
echo "-------------------------------------------------------------"
export LANG=ja_JP.sjis
cat test_sjis.txt|./iconv_Chk 2
export LANG=ja_JP.utf8
---------------------------------------------------------------
*/
main(int argc, char **argv)
{
    int ret;
    char *moji;
    char *moji_to;
      
    /*ロケールの設定*/
    char *loc;
   //loc = setlocale( LC_ALL, "ja_JP.sjis" );
   loc = setlocale( LC_ALL, "" );  /*システムのlocale設定を使用する*/
   printf("LOCALE====%s\n",loc);


   /*引数の判定*/
   int proc = atoi(argv[1]);
   //1:utf8-->cp932
   //2:cp932-->utf8
   char *ifrom="";
   char *ito="";    
   switch(proc){
      case 1:
         loc = setlocale( LC_ALL, "ja_JP.utf8" ); /*ロケールをUTF8に変更*/
         ifrom="UTF-8";
         ito="cp932";
         break;
      case 2:
         loc = setlocale( LC_ALL, "ja_JP.sjis" ); /*ロケールをSJISに変更*/
         ifrom="cp932";
         ito="UTF-8";
         break;
      default:
         break;
   }


   /* 標準入力からデータを取得（パイプ）-----------------------------------S
   */
   char *buf;
   char *buf2;
   int r=2;
   int inMojiSize=0;
    buf = (char *)calloc(READ_SIZE,sizeof(char));
    buf2 = (char *)calloc(READ_SIZE,sizeof(char));

      while (fgets(buf,READ_SIZE,stdin) != NULL){
         /* 容量が足りなくなったら再確保 */
         buf2 = (char *)realloc(buf2,READ_SIZE * r  );
         /* １行又は指定バイト単位で読み込んだデータを蓄積 */
         strcat(buf2,buf);
         r++;
      }
      moji = buf2;
      /*入力のサイズ取得*/
      inMojiSize = strlen(moji);
   /* 標準入力からデータを取得（パイプ）-----------------------------------E
   */

    int i;
    int cnt;
    char *remoji[inMojiSize];
    char *retstr[inMojiSize];

    /*１文字ずつ配列に格納する*/
    chr_conv(moji,remoji,inMojiSize);
puts("***************************");
    for(cnt=0;remoji[cnt]!='\0';cnt++){

#ifdef DEBUG1
printf("--変換前[%02s]  cnt:%d  ",remoji[cnt],cnt);
#endif
       /*UTF8->cp932に変換（１文字ずつ）*/
       ret = convert(ifrom, ito,
                     remoji[cnt],
                     outbuf, sizeof(outbuf));
       if (ret) {
#ifdef DEBUG1
printf("変換後[%02s]  \n", outbuf);
#endif
       }
       else {
           /*変換不能文字があった場合「!NG!」に変換*/
           strcpy(outbuf,DAMEMOJI);
#ifdef DEBUG1
printf("変換後[%02s]  \n", outbuf);
#endif
       }
       /*１文字ずつをつなぎ合わせる*/
       strcat((char *)retstr,outbuf);
   }
   puts((char *)retstr);
#ifdef DEBUG1
printf("１文字変換後>---len--:%d  [%s]\n",strlen(retstr),retstr);
puts("\n----------------１文字convert-------------------E");
#endif
puts("***************************");

   free(buf);
   free(buf2);
}

/*
---------------------------------------------------------------
引数で渡された文字列「char const *str」を
1文字ずつにして配列「char *hairtetu[]」に格納して返却する
---------------------------------------------------------------
*/
void chr_conv(char const *str,char *hairtetu[],int cnt){
   int i=0;
   int j,len;
   int h=0;
   char *rep;
   rep = (char *)malloc(sizeof(char) * cnt );

   while(str[i]!='\0'){
      /*マルチバイト文字のバイト数を取得（1文字）*/
      len= mblen( &str[i], MB_CUR_MAX );
      /*ワークをクリア*/
      for(j=0;j<cnt;j++){
         rep[j]='\0';
      }
      /*文字数を指定してコピー*/
      strncpy(rep,str+i,len);
      if(len == 1){
         /*シングルバイト文字*/
      }
      else{
         /*マルチバイト文字*/
      }

      /*配列要素を確保して１文字入れる*/
      hairtetu[h]=malloc(sizeof(rep));
      strcpy(hairtetu[h],rep);
      h++;

      i+=len;
   }
      /*最終位置の設定*/
      hairtetu[h]=malloc(sizeof(rep));
      hairtetu[h]='\0';
   
      free(rep);
}
/*
---------------------------------------------------------------
文字コード変換処理。iconvを利用
---------------------------------------------------------------
*/
int convert(char const *src,
            char const *dest,
            char const *intext,
            char *outbuf,
            size_t bufsize)
{
    iconv_t cd;
    size_t insrclen, outsrclen;
    size_t ret;

    cd = iconv_open(dest, src);
    if (cd == (iconv_t)-1) {
#ifdef DEBUG1
perror("iconv open");
#endif
        return 0;
    }

    insrclen = strlen(intext);
    outsrclen = bufsize - 1;
    memset(outbuf, '\0', bufsize);

    ret = iconv(cd, &intext, &insrclen, &outbuf, &outsrclen);
    if (ret == -1) {
#ifdef DEBUG1
perror("iconv");
#endif
        return 0;
    }

    iconv_close(cd);
    return 1;
}