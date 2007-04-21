

ret = []
Dir.glob("*.{h,cpp}"){ |file|
	ret |= IO.read(file).scan(/(?:Xid|Xm)\((.*?)\)/).select{|it| not it=~ /[^a-zA-Z0-9_]/ }.flatten
}

src = IO.read("string.h")
open("string.h","w"){ |out|
	out << src.gsub(/\/\/{{{.*?\/\/}}}/m){
		str = "//{{{\n"
		str << "enum{"
		ret.each{ |it|
			str << "\tid#{it},\n"
		}
		str << "};"
		str << "//}}}\n"
	}
}

src = IO.read("string.cpp")
open("string.cpp","w"){ |out|
	out << src.gsub(/\/\/{{{.*?\/\/}}}/m){
		str = "//{{{\n"
		str << "const ID& get_id(int_t n){\n"
		ret.each{ |it|
			
		}
		str << "}\n"
		str << "//}}}\n"
	}
}