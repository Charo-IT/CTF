#coding:ascii-8bit

def rotate_left(b, w)
  b &= 0xff
  ((b << w) | (b >> (8 - w))) & 0xff
end

def rotate_right(b, w)
  b &= 0xff
  ((b >> w) | (b << (8 - w))) & 0xff
end

key = [67, 65, 204, 214, 142, 225, 48, 135, 216, 218, 230, 196, 49, 185, 84, 227, 145, 45, 8, 114, 179, 179, 36, 15, 96, 68, 113, 48, 23, 212, 121, 34, 48, 162, 151, 164, 175, 56, 39]

f = ""
k = 0
key.each_with_index{|b, i|
  f << rotate_right(k + b, i % 8).chr
  k = b
}
puts f
