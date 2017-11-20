#coding:ascii-8bit
require "open3"

def test(program)
  stdin, stdout, thread = Open3.popen2("./fs_hell /dev/stdin")
  stdin.write(program)
  stdin.close
  while thread.alive?
    print stdout.read
  end
end

def rotate_left(b, w)
  b &= 0xff
  ((b << w) | (b >> (8 - w))) & 0xff
end

def rotate_right(b, w)
  b &= 0xff
  ((b >> w) | (b << (8 - w))) & 0xff
end

def encode(str)
  k = 0
  str.bytes.each_with_index.map{|b, i|
    k = ((rotate_left(b, i % 8) - k) & 0xff)
  }
end

def src_reg
  {ax: 1, bx: 2, cx: 3, px: 4, mem: 5, zflag: 11, sflag: 12}
end

def dst_reg
  {ax: 6, bx: 7, cx: 8, px: 9, mem: 10, delta_pc: 13}
end

def load(src)
  case
  when src.is_a?(Integer)
    "%14$.#{src & 0xffff}d"
  when src.is_a?(Symbol)
    "%14$.*#{src_reg[src]}$d"
  else
    raise
  end
end

def store(dst, byte = false)
  "%#{dst_reg[dst]}$#{byte ? "h" : ""}hn"
end

def mov(dst, src, byte = false)
  load(src) + store(dst, byte)
end

# dst = src.inject(:+)
def add(dst, src, byte = false)
  src.map{|a| load(a)}.join + store(dst, byte)
end

# if ax == 0
#   pc += 2
# else
#   pc += 1
def jz
  load(:zflag) + store(:delta_pc)
end

# if ax < 0
#   pc += 2
# else
#   pc += 1
def js
  load(:sflag) + store(:delta_pc)
end

def jmp(diff)
  load(diff - 1) + store(:delta_pc)
end

flag = "CBCTF{Do_Y0U_W4nt_MOR3_foRm4t_57RiNg5?}"
keys = encode(flag)
var_input = 0
var_keys = 0x120
var_result = 0x100
var_k = 0x102
var_i = 0x104
var_j = 0x106  # i % 8
var_l = 0x108  # for shift
var_tmp = 0x10a
var_minus = 0x200
label = {}

program = ""
scrambled = keys.each_with_index.map{|k, i|
  if Random.rand(0..1) == 0
    v = Random.rand(1...k)
    [[v, i], [k - v, i]]
  else
    [k, i]
  end
}.flatten.each_slice(2).to_a.shuffle
px = 0
program << mov(:px, var_keys) + "\n"
scrambled.each{|k, i|
  program << add(:px, [:px, i - px]) + "\n"
  program << add(:mem, [:mem, k], true) + "\n"
  px = i
}

program << "\n"

# generate lookup table
program << mov(:ax, 0xff) + "\n"
program << mov(:px, var_minus + 1) + "\n"
program << js + "\n"
program << jmp(2) + "\n"  # if ax >= 0
program << jmp(5) + "\n"  # if ax < 0
program << mov(:mem, :ax, true) + "\n"
program << add(:px, [:px, 1]) + "\n"
program << add(:ax, [:ax, -1]) + "\n"
program << jmp(-6) + "\n"
program << "\n"

# check
program << mov(:px, var_result) + "\n"  # result = 0
program << mov(:mem, 0) + "\n"
program << mov(:px, var_k) + "\n"  # k = 0
program << mov(:mem, 0) + "\n"
program << mov(:px, var_i) + "\n"  # i = 0
program << mov(:mem, 0) + "\n"
program << mov(:px, var_j) + "\n"  # j = 0
program << mov(:mem, 0) + "\n"

program << mov(:px, var_i) + "\n"  # test i == keys.length
program << add(:ax, [:mem, -keys.length]) + "\n"
program << jz + "\n"
program << jmp(2) + "\n" # if ax != keys.length
program << jmp(44) + "\n" # if ax == keys.length

# rotate
program << mov(:px, var_i) + "\n"  # bx = s[i]
program << mov(:px, :mem) + "\n"
program << mov(:bx, :mem) + "\n"
program << mov(:px, var_j) + "\n"  # ax = j
program << mov(:ax, :mem) + "\n"
program << jz + "\n"  # test j == 0
program << jmp(2) + "\n"  # if j != 0
program << jmp(4) + "\n"  # if j == 0
program << add(:bx, [:bx, :bx]) + "\n"  # bx <<= 1
program << add(:ax, [:ax, -1]) + "\n"  # ax -= 1
program << jmp(-5) + "\n"
program << mov(:px, var_tmp) + "\n"  # tmp = bx
program << mov(:mem, :bx) + "\n"
program << mov(:ax, :mem) + "\n"  # ax = rotate_left(s[i], i % 8)
program << add(:px, [:px, 1]) + "\n"
program << add(:ax, [:ax, :mem]) + "\n"

program << mov(:px, var_k) + "\n"
program << add(:px, [:mem, var_minus]) + "\n"
program << add(:ax, [:ax, :mem], true) + "\n"  # ax = rotate_left(s[i], i % 8) - k

# check byte
program << mov(:px, var_i) + "\n"
program << add(:px, [:mem, var_keys]) + "\n"
program << mov(:bx, :mem) + "\n"
program << mov(:px, var_k) + "\n"  # k = keys[i]
program << mov(:mem, :bx) + "\n"
program << add(:px, [:bx, var_minus]) + "\n"
program << add(:ax, [:ax, :mem], true) + "\n"
program << mov(:px, var_result) + "\n"
program << jz + "\n"
program << jmp(2) + "\n"  # ax != keys[i]
program << jmp(3) + "\n"  # ax == keys[i]
program << mov(:mem, 1) + "\n"
program << jmp(3) + "\n"
program << mov(:mem, :mem) + "\n"
program << mov(:ax, :ax) + "\n"

# increment counter
program << mov(:px, var_i) + "\n"
program << add(:mem, [:mem, 1]) + "\n"
program << mov(:px, var_j) + "\n"
program << add(:mem, [:mem, 1]) + "\n"
program << add(:ax, [:mem, -8]) + "\n"
program << jz + "\n"
program << jmp(2) + "\n"  # j != 7
program << mov(:mem, 0) + "\n"  # j == 7
program << jmp(-47) + "\n"

program << mov(:px, var_result) + "\n"
program << mov(:ax, :mem) + "\n"

puts program
open("program.txt", "wb"){|f| f.write program}
