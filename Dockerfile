FROM ubuntu:latest
ARG milk_branch
RUN useradd -ms /bin/bash milkuser
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
    apt-get install -y \
        git \
	make \
	dpkg-dev \
	libc6-dev \
        cmake \
        pkg-config \
        python3-dev \
        libcfitsio-dev \
        pybind11-dev \
        python3-pybind11 \
        libfftw3-dev \
        libncurses-dev \
        libbison-dev \
        libfl-dev \
        libreadline-dev \
	pkg-config \
	gcc-10 \
	g++-10

RUN rm /usr/bin/gcc /usr/bin/g++ && \
    ln /usr/bin/gcc-10 /usr/bin/gcc && \
    ln /usr/bin/g++-10 /usr/bin/g++

RUN git clone -b $milk_branch https://github.com/milk-org/milk.git /build
WORKDIR /build
ENV MILK_PYTHON="OFF"
RUN bash ./compile.sh

WORKDIR /build/_build
RUN make install

RUN mkdir /work
WORKDIR /work
RUN ln -s /usr/local/milk-* /usr/local/milk
ENV MILK_ROOT /build
ENV MILK_INSTALLDIR /usr/local/milk
ENV PATH ${PATH}:${MILK_INSTALLDIR}/bin
ENV PKG_CONFIG_PATH $PKG_CONFIG_PATH:${MILK_INSTALLDIR}/lib/pkgconfig
USER milkuser
