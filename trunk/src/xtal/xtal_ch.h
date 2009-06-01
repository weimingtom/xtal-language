/** \file src/xtal/xtal_ch.h
* \brief src/xtal/xtal_ch.h
*/

#ifndef XTAL_CH_H_INCLUDE_GUARD
#define XTAL_CH_H_INCLUDE_GUARD

#pragma once

namespace xtal{

uint_t edit_distance(const void* data1, uint_t size1, const void* data2, uint_t size2);

/**
* \brief �}���`�o�C�g������g�ݗ��Ă邽�߂̃��[�e�B���e�B�N���X
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

#endif // XTAL_CH_H_INCLUDE_GUARD