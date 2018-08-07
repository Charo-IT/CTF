#coding:ascii-8bit
require "fiddle/import"

SYS_execve = 59
SYS_fork = 57

module Seccomp
  extend Fiddle::Importer
  dlload "libseccomp.so.2"
  extern "void *seccomp_init(int)"
  extern "int seccomp_rule_add(void *, unsigned int, int, unsigned int)"
  extern "int seccomp_load(void *)"

  SCMP_ACT_KILL = 0
  SCMP_ACT_ALLOW = 0x7fff0000
end

def apply_seccomp
  result = 0
  ctx = Seccomp.seccomp_init(Seccomp::SCMP_ACT_ALLOW)
  result |= Seccomp.seccomp_rule_add(ctx, Seccomp::SCMP_ACT_KILL, SYS_execve, 0)
  result |= Seccomp.seccomp_rule_add(ctx, Seccomp::SCMP_ACT_KILL, SYS_fork, 0)
  result |= Seccomp.seccomp_load(ctx)

  raise "seccomp failed" unless result == 0
end

puts <<EOS
Hi, there :)

I made a sandbox for you.
Show me if you can break this and read /home/user/flag.

Environment:
  * Ubuntu 16.04 x86_64 latest
  * Ruby 2.2.10 (compiled as PIE)
EOS

sleep 1
puts
puts

puts <<EOS
Okay, let's begin ;)
Don't forget to put the line "__END__" to the end of your script!
EOS
puts

script = ""
while true
  print "> "
  s = STDIN.gets
  if s.chomp == "__END__"
    break
  end
  script << s
end

STDIN.close
ARGV.freeze
ObjectSpace.each_object(Module, &:freeze)

apply_seccomp
STDOUT.puts Thread.new(script){|s| eval "$SAFE=3;#{s}"}.value
