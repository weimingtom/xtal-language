
#pragma once

namespace xtal{
	
/**
* @brief 先頭バイトを見て、そのマルチバイト文字が何文字かを調べる。
*
* マイナスの値が返された場合、最低文字数を返す。
* -2の場合、最低2文字以上の文字で、本当の長さは2文字目を読まないと判断できない、という意味となる。
*/
int_t ch_len(char_t lead);

/**
* @brief マルチバイト文字が何文字かを調べる。
*
* int_t ch_len(char_t lead)が呼ばれた後、マイナスの値を返した場合に続けて呼ぶ。
* ch_lenで-2の値を返した後は、strの先には最低2バイト分のデータを格納すること。
*/
int_t ch_len2(const char_t* str);


/**
* @brief 一つ先の文字を返す
*
* 例えば a を渡した場合、b が返る
*/
StringPtr ch_inc(const char_t* data, int_t data_size);

/**
* @brief 文字の大小判定
*
* 負の値 a の文字の方がbの文字より小さい
* 0の値 等しい
* 正の値 bの文字の方がaの文字より小さい
*/
int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);

uint_t edit_distance(const void* data1, uint_t size1, const void* data2, uint_t size2);

/**
* @brief マルチバイト文字を組み立てるためのユーティリティクラス
*/
class ChMaker{
public:

	ChMaker(){
		pos_ = 0;
		len_ = -1;
	}

	bool is_completed(){
		return pos_==len_;
	}

	void add(char_t ch){
		buf_[pos_++] = ch;
		if(pos_==1){
			len_ = ch_len(ch);
		}
		else if(pos_ == -len_){
			len_ = ch_len2(buf_);
		}
	}

	const IDPtr& to_s(){
		return temp_ = xnew<ID>(&buf_[0], pos_);
	}

	int_t pos(){
		return pos_;
	}

	char_t at(int_t i){
		return buf_[i];
	}

	const char_t* data(){
		return buf_;
	}

	void clear(){
		pos_ = 0;
		len_ = -1;
	}

private:

	int_t pos_;
	int_t len_;
	char_t buf_[8];
	IDPtr temp_;
};

}
