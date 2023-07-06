/*
 * HandyGraphicを利用した上から落ちてくる文字をタイプするゲーム
 *
 * 2023/07/05 Kawa_09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <handy.h>

#define WND_WIDTH 600
#define WND_HEIGHT 600
#define KANA_NUM 85
#define SMALL_KANA_NUM 16
#define SMALL_KANA_FIRST_NUM 85
#define SMALL_KANA_LAST_NUM 105
#define JPN_CHAR_U 2
#define JPN_CHAR_KU 7
#define JPN_CHAR_SI 11
#define JPN_CHAR_TI 16
#define JPN_CHAR_HU 27
#define JPN_CHAR_ZI 51
#define JPN_CHAR_VU 72
#define JPN_CHAR_LTU 97
#define JPN_CHAR_NN 105
#define JPN_CHAR_BAR 106
#define FINISH_TYPING 2

/* ------ 構造体の宣言 ------*/
// 文字列の管理をする構造体
typedef struct{
    int canDraw; // 描画したかどうかを保持する変数
    double x; // 描画時のx座標を保持する変数
    double y; // 描画時のy座標を保持する変数
    int inNum[2]; // 何文字まで入力されたのかを保存する変数.　[0]:何文字目、[1]:[0]の何文字目かを表す
    char origin[256]; // 落とす文字列を保存する配列
    char kana[256]; // 落とす文字列の仮名を保存する配列
    char example[128]; // ローマ字の入力例を保存する配列
    char wait[20][10][128]; // 入力待ちの文字のパターンを保存する配列
}Str;

/* ------ プロトタイプ宣言 ------ */
int random_string_index(int strNum, Str *strings); // 文字列の個数内の乱数を返す関数
void set_string_example(Str *strings, int strIndex); // ローマ字で各文字と全文の入力例をセットする関数
int set_char_pattern(Str *strings, int strIndex, int charIndex, int japaneseCharIndex); // 文字の入力パターンをセットする関数
int get_japanese_index(char *str, int charIndex); // 対応している日本語の文字を、対応するローマ字が保存されている配列の添え字を返す
int check_input_char(Str *strings, int strIndex, unsigned int ch); // 入力された文字の正誤判定をし、場合によって入力例を書き換える

/* ------ グローバル変数の宣言 ------*/
// 母音を保管する配列
char vowel[][2] = {"a","i","u","e","o"};

// 子音を保管する配列
char consonant[][3][4] = {{""},{"k"},{"s","sh"},{"t","ch"},{"n"},
                          {"h","f"},{"m"},{"y"},{"r"},{"g"},
                          {"z","j"},{"d"},{"b"},{"p"},{"v"},{"w"},{"wy"},
                          {"l","x"},{"ly","xy"},{"lt"},{"lw","xw"},
                          {"n","nn"}};

// 拗音がくるパターンを保存する二次元配列
int youon[KANA_NUM][SMALL_KANA_NUM];

// 拗音に対応するための文字列を保管する配列
char youonStr[][3][4] = {{""},{"y"},{"w"},{"h"},{"y","h"},
                         {"w","q"},{"f"},{"f","y"},{"v"},{"wh"},
                         {"wh","w"}};

// ひらがなと伸ばし棒のデータを保管する配列
char japaneseStr[] = "あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやいゆえよらりるれろ"
                     "がぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽあいゔえおわいうえをあゐうゑおぁぃぅぇぉゃぃゅぇょぁぃっぇぉゎぃぅぇぉんー";

/* ---------------------- */
/* ------ メイン処理 ------ */
/* ---------------------- */
int main() {

    /* ------ HandyGraphic関係の変数の宣言 ------ */
    int titleLayerId; // タイトル用のレイヤのidを保存する変数
    double romajiX,romajiY; // 入力例文字列の描画範囲を保存するための変数
    doubleLayer doubleLayerId; // ダブルレイヤ変数の宣言
    hgevent *eventCtx = NULL; // HgEventの返り値のポインタを保存するhgevent型ポインタ変数

    /* ------ タイピングの処理用の変数の宣言 ------ */
    int strNum = 0; // 落とす文字列の数を保存する変数
    int strIndex; // 落とす文字列の配列の番号を保存する変数
    Str *strings = NULL; // 文字列の情報を保持する構造体

    /* ------ ゲームのシステムに関係する変数の宣言 ------ */
    int level = 0; // 難易度を表す仮の変数

    /* ------ ファイルポインタの宣言 ------ */
    FILE *fpInYouon; // 拗音がくるパターンのあるファイル用のポインタ
    FILE *fpInString; // 落とす文字列のあるファイル用のポインタ
    FILE *fpInStringKana; // 落とす文字列の仮名のあるファイル用のポインタ

    /* ------ 構造体のメモリを動的に確保する ------ */
    strings = (Str*) malloc(100 * sizeof(Str));

    /* ------- テキストファイルの読み込み ------- */
    // 拗音のパターンのあるファイルを開く
    if((fpInYouon = fopen("./../youon.txt","r")) == NULL){
        printf("ファイルのオープンに失敗しました\nyouon.txtがあるかを確認してください\n");
        exit(0);
    }
    if((fpInString = fopen("./../string.txt","r")) == NULL){
        printf("ファイルのオープンに失敗しました\nstring.txtがあるかを確認してください\n");
        exit(0);
    }
    if((fpInStringKana = fopen("./../string_kana.txt","r")) == NULL){
        printf("ファイルのオープンに失敗しました\nstring_kana.txtがあるかを確認してください\n");
        exit(0);
    }

    // 拗音のパターンをファイルから取得
    for(int i = 0; i < KANA_NUM; i++)for(int j = 0; j < SMALL_KANA_NUM; j++)fscanf(fpInYouon,"%d", &youon[i][j]);
    // 落とす文字列とその仮名をファイルから取得
    for(int i = 0; i < 100; i++){
        if(fscanf(fpInString, "%s", strings[i].origin) == EOF){
            break;
        }
        fscanf(fpInStringKana, "%s",strings[i].kana);
        strings[i].canDraw = 0;
        strings[i].x = 200.0;
        strings[i].y = 600.0;
        strings[i].inNum[0] = 0;
        strNum++; // 文字列の数を数える
        for(int j = 0; j < 10; j++){
            sprintf(strings[i].wait[i][j] ,"%s", "");
        }
    }

    // Windowを開く
    HgOpen(WND_WIDTH,WND_HEIGHT);

    // タイトル用のレイヤを追加する
    titleLayerId = HgWAddLayer(0);

    // タイトルの描画
    // タイトルのデザイン用の設定、描画をする
    HgWSetFont(titleLayerId,HG_M,50); // フォントとサイズを設定
    HgWText(titleLayerId,165,500,"Fall Typing"); // タイトルの文字列を描画
    HgWBox(titleLayerId,200,100,200,100);
    HgWBox(titleLayerId,200,225,200,100);
    HgWBox(titleLayerId,200,350,200,100);

    // マウスのクリックを検知し、ゲームモードを設定する
    HgSetEventMask(HG_MOUSE_DOWN); // イベントマスクをマウスのクリックで設定する

    // levelの値が0の間ループする
    do {
        eventCtx = HgEvent(); // イベントを取得する

        // 描画されたボックスの位置をクリックした時、難易度を設定する
        if(200 <= (*eventCtx).x && (*eventCtx).x <= 400){
            if(350 <= (*eventCtx).y && (*eventCtx).y <= 450){
                level = 1;
            }else if(225 <= (*eventCtx).y && (*eventCtx).y <= 325){
                level =2;
            }else if(100 <= (*eventCtx).y && (*eventCtx).y <= 200){
                level = 3;
            }
        }
    }while(level == 0);
    printf("難易度%dが選択されました\n", level);
    printf("x: %lf y: %lf\n", (*eventCtx).x, (*eventCtx).y);

    // タイトルレイヤを非表示にする
    HgClear();

    // ダブルレイヤを作成する
    doubleLayerId = HgWAddDoubleLayer(0);
    /*
     * まだ選ばれていないテキストをランダムに選択
    * 速度と文字数から次に出す難易度を算出する
    */
    if((strIndex = random_string_index(strNum, strings)) == -1){
        printf("これ以上出力できる文字列がありません\n");
        strIndex = 0;
    }
    // 入力例文字列の設定
    set_string_example(strings,strIndex);

    // キー入力が得られるようにマスクを設定
    HgSetEventMask(HG_KEY_DOWN);
    for(;;) { // 無限ループ
        // 表示するレイヤに関する処理
        int layerId = HgLSwitch(&doubleLayerId);
        HgLClear(layerId); // レイヤの描画を削除する

        // 落とす場所もすでに落としている文字列に被らないようにランダムに決める
        /* ------ 描画 ------ */
        // 画面の装飾
        HgSetColor(HG_RED);
        HgLine(0, 150, 600, 150);
        // 文字列の描画
        HgSetColor(HG_BLACK);

        // 入力が終わっていなかったら文字列を描画する
        if (strings[strIndex].canDraw != 2) {
            HgWText(layerId, 200, strings[strIndex].y, strings[strIndex].origin); // 文字列を描画
            HgWSetFont(layerId, HG_M, 50);
            HgWTextSize(layerId, &romajiX, &romajiY, strings[strIndex].example); // 入力例文字列の描画範囲を取得
            HgWText(layerId, WND_WIDTH / 2.0 - romajiX / 2.0, 150 / 2.0 - romajiY / 2.0,
                    strings[strIndex].example); // 文字列の描画
            HgWSetFont(layerId, HG_M, 30);
        }

        // 落ちてくる文字がラインについたら終了
        if(strings[strIndex].y < 150){
            break;
        }

        // 動かす
        strings[strIndex].y -= 1.0; // 仮の文字列を仮の速度で下に落とす
        // 入力の常時受けとり
        eventCtx = HgEventNonBlocking(); // イベントを取得する
        if(eventCtx != NULL){// イベントがあった時
            if(eventCtx->type == HG_KEY_DOWN){ // イベントがキー入力の時
                // 正誤判定とそれの反映の準備
                check_input_char(strings,strIndex,eventCtx->ch);
                // タブキーで入力する文字を選択
            }
        }

        // 今洗濯している文字列が入力終了しているかを判定
//        if(strings[strIndex].example[strings[strIndex].inNum[0]] == '\0' && strings[strIndex].canDraw == 1){
//            // スコアの処理
//            // 描画を終了する
//            strings[strIndex].canDraw = 2;
//        }
        // 落下スピードも調整してもいいかも？
        // スコアの計算
        // レベル（難易度）の概念を持たせて、場の単語の数を管理する
        HgSleep(0.02); // 少し処理を止める
    }
    //--------------------------
    // 結果と最終スコアを表示
    // 終わり

    // Windowを閉じる
    HgClose();

    return 0;
}

/* ---------------------- */
/* ------ ユーザ関数 ------ */
/* ---------------------- */

// これまで表示されていない文字列配列のindexをランダムに返す
int random_string_index(int strNum, Str *strings){
    int canCheck = 0; // 表示できる文字列が残っているかどうかを保持する変数
    int random; // 乱数を保存する変数

    for(int i = 0; i < strNum; i++){
        if(strings[i].canDraw == 0){ // 表示できる文字列があった時
            canCheck = 1; // check変数に値を入れて、ループを抜ける
            break;
        }
    }
    // 表示できる文字列がなかった時、エラーを返す
    if(canCheck == 0){
        return -1;
    }

    // ランダムにこれまで表示していない文字列の番号を探す
    srand((unsigned int)time(NULL)); // 乱数の初期化
    do{
        random = rand()% strNum; // 0 ~ strNum までの乱数を出力
    }while(strings[random].canDraw == 1);
    strings[random].canDraw = 1; // 選んだのでマークする

    return random;
}

/**
 * ローマ字で各文字の入力例と全文の入力例をセットする関数
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 **/
void set_string_example(Str *strings, int strIndex){
    int len = strlen(strings[strIndex].kana); // 文字列の長さを保存する変数
    int nowCharIndex; // 文字の番号を保存する変数
    int nextCharIndex; // 一つ先の位置の文字の番号を保存する変数
    int charArrayIndex; // 文字のセットされている配列の数を保存する変数
    int youonNum = -1; // 拗音のパターンを表す変数

    // 文字のバイト数が3なので、3ずつプラスしてループする
    /*
     * nowCharIndex / 5 : 母音の数を割ることで、 子音の番号と合わせる
     * nextCharIndex % 5: 剰余算をする事で、母音の番号と合わせる
     * */
    printf("%s %d\n", strings[strIndex].kana, len);
    for(int i = 0; i < len; i+=3){
        nowCharIndex = get_japanese_index(strings[strIndex].kana,i);
        charArrayIndex = set_char_pattern(strings,strIndex,i,nowCharIndex); // 文字の入力パターンをセットする
        if(i+3 > len)break; // 次の文字がない時は終了
        nextCharIndex = get_japanese_index(strings[strIndex].kana,i+3);
        if(nextCharIndex >= SMALL_KANA_FIRST_NUM && nextCharIndex < SMALL_KANA_LAST_NUM) { // 次の文字が小書き文字なら
            // 添字の番号を調整して、拗音のパターンの数字を代入
            youonNum = youon[nowCharIndex][nextCharIndex - SMALL_KANA_FIRST_NUM];
        }

        if(youonNum > 0 && nextCharIndex != JPN_CHAR_LTU){ // youonNum > 0 : 拗音であることを表す
            if(nowCharIndex == JPN_CHAR_U) {
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%c", youonStr[youonNum][0],
                        vowel[nextCharIndex % 5], '*');
                if(youonNum == 10){
                    charArrayIndex++;
                    sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%c", youonStr[youonNum][1],
                            vowel[nextCharIndex % 5], '*');
                }
            }else if(nowCharIndex == JPN_CHAR_KU){
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                charArrayIndex++;
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%c", youonStr[youonNum][1],
                        vowel[nextCharIndex % 5],'*');
            }else if(nowCharIndex == JPN_CHAR_TI) {
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                if (youonNum == 4)
                    charArrayIndex++;
                    sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][1],
                                          youonStr[youonNum][1], vowel[nextCharIndex % 5],'*');
            }else if(nowCharIndex == JPN_CHAR_SI){
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                charArrayIndex++;
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][1], vowel[nextCharIndex % 5],'*');
            }else if(nowCharIndex == JPN_CHAR_HU || nowCharIndex == JPN_CHAR_VU){
                if(youonNum == 6 || youonNum == 8){
                    sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%c", youonStr[youonNum][0],
                            vowel[nextCharIndex % 5],'*');
                }else if(youonNum == 7){
                    sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%c", youonStr[youonNum][0],
                            vowel[nextCharIndex % 5],'*');
                    charArrayIndex++;
                    sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                            youonStr[youonNum][1], vowel[nextCharIndex % 5],'*');
                }else{
                    sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                            youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                }
            }else if(nowCharIndex == JPN_CHAR_ZI){
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                charArrayIndex++;
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%c%s%c", 'j', vowel[nextCharIndex % 5],'*');
            }else{
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
            }
            charArrayIndex++;
        }else if(nowCharIndex == JPN_CHAR_LTU) {
            if(5 < nextCharIndex){ // 次の文字が母音意外だった時
                sprintf(strings[strIndex].wait[i][charArrayIndex], "%c%s%s%c",
                        consonant[nextCharIndex / 5][0][0], consonant[nextCharIndex / 5][0],vowel[nextCharIndex % 5],'*');
            }
        }else if(nowCharIndex == JPN_CHAR_NN) {
            // 次の文字あり、母音でないかつ、や行出なかった時、「n」をセットする
            if (nextCharIndex > 5 &&
                !(20 <= nextCharIndex && nextCharIndex < 25) &&
                !(35 <= nextCharIndex && nextCharIndex < 40) ) {
                sprintf(strings[strIndex].wait[i][1], "%s", "n");
            }
        }
        youonNum = -1;

    }

    int j;
    for(int i = 0; i < len; i += 3){
        for(j = 0; j < 10; j++){
            if(strings[strIndex].wait[i][j][0] == '\0')break;
            printf("%d:%s\n", j,strings[strIndex].wait[i][j]);
        }
        sprintf(strings[strIndex].example, "%s%s", strings[strIndex].example, strings[strIndex].wait[i][j-1]);
        if(strings[strIndex].example[strlen(strings[strIndex].example)-1] == '*'){
            strings[strIndex].example[strlen(strings[strIndex].example)-1] = '\0';
            i += 3;
        }
    }
}

/**
 * 指定された日本語の１文字の入力パターンを作成する
 * 「っ」は「ltu」のパターンのみを作成する
 * 「ん」は「nn」のパターンのみを作成する
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 * @param charIndex 文字列の何文字目かを指定する番号
 * @param japaneseCharIndex 日本語の文字のを指定する変数
 *
 * @return セットされた配列の数を返す
 *
 */
int set_char_pattern(Str *strings, int strIndex, int charIndex, int japaneseCharIndex) {

    if(0 <= japaneseCharIndex && japaneseCharIndex < 5){
        sprintf(strings[strIndex].wait[charIndex][0], "%s", vowel[japaneseCharIndex]);
        return 1;
    }else if(SMALL_KANA_FIRST_NUM <= japaneseCharIndex && japaneseCharIndex < SMALL_KANA_LAST_NUM ||
       japaneseCharIndex == JPN_CHAR_SI ||
       japaneseCharIndex == JPN_CHAR_TI ||
       japaneseCharIndex == JPN_CHAR_HU ||
       japaneseCharIndex == JPN_CHAR_ZI) {
        sprintf(strings[strIndex].wait[charIndex][0], "%s%s", consonant[japaneseCharIndex / 5][0], vowel[japaneseCharIndex % 5]);
        sprintf(strings[strIndex].wait[charIndex][1], "%s%s", consonant[japaneseCharIndex / 5][1], vowel[japaneseCharIndex % 5]);
        return 2;
    }else if(japaneseCharIndex == JPN_CHAR_NN) {
        sprintf(strings[strIndex].wait[charIndex][0], "%s", consonant[japaneseCharIndex / 5][1]);
        return 1;
    }else if(japaneseCharIndex == JPN_CHAR_BAR) {
        strcat(strings[strIndex].wait[charIndex][0], "-");
        return 1;
    }else{
        sprintf(strings[strIndex].wait[charIndex][0], "%s%s", consonant[japaneseCharIndex / 5][0], vowel[japaneseCharIndex % 5]);
        return 1;
    }
}


/**
 * 指定された日本語の文字の番号を返す
 *
 * @param str 日本語の文字が保存されている配列
 * @param charIndex 日本語の文字を指定する番号
 *
 * @return 日本語の文字の番号を返す
 */
int get_japanese_index(char *str, int charIndex){

    for(int i = 0; i < strlen(japaneseStr); i+=3){
        if(strncmp(&str[charIndex], &japaneseStr[i], 3) == 0){
            return i/3;
        }
    }
    return -1;
}

/**
 * 入力された文字の正誤判定を行い、入力例と違うが間違いでない入力だった時に入力例を書き換える。
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 * @param ch 入力されたアルファベット一文字
 *
 * @return 0:成功 -1:失敗 で入力の正誤を返す
 */
int check_input_char(Str *strings, int strIndex, unsigned int ch) {

    if(strings[strIndex].example[strings[strIndex].inNum[0]] == ch){ // 入力例と同じだった時
        strings[strIndex].inNum[0]++;
        return 0;
    }else{ // 入力例と違った時
        if(strings[strIndex].example[strings[strIndex].inNum[0]] == '\0'){ // 入力例の文字列の最後まで入力された時
            strings[strIndex].inNum[0]++;
            return 0;
        }else if(strings[strIndex].example[strings[strIndex].inNum[0]+1] == ch){ // 入力例の一つ先の文字と同じだった時
            strings[strIndex].inNum[0]++;
            return 0;
        }else{ // それ以外
            return -1;
        }

    }

    return -1;
}