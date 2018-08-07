FROM ubuntu:16.04

ADD little_riddle.rb /tmp
ADD flag /tmp

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y wget gcc make zlib1g-dev libssl-dev && \
    cd /tmp && \
    wget https://cache.ruby-lang.org/pub/ruby/2.2/ruby-2.2.10.tar.gz && \
    tar zxvf ruby-2.2.10.tar.gz && \
    cd ruby-2.2.10 && \
    ./configure && make CFLAGS='${cflags} -pie -fPIC' && make install && \
    cd /tmp && \
    rm -rf ruby-2.2.10* && \
    gem update --system && \
    gem update && \
    useradd -m user && \
    mv little_riddle.rb /home/user/ && \
    chown -R root:user /home/user && \
    chmod 750 /home/user && \
    mv /tmp/flag /home/user/flag && \
    chown root:user /home/user/flag && \
    chmod 440 /home/user/flag /home/user/little_riddle.rb

CMD ["su", "user", "-c", "timeout 60 ruby /home/user/little_riddle.rb"]
