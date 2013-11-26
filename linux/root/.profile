. $HOME/.aliases

export BASH_ENV=$HOME/.bashrc
export ENV=$HOME/.profile
export EDITOR=pico
export PS1="\h:\u[\t]#"

export PATH=.:~/bin:/usr/local/bin:/usr/local/sbin:/sbin:/usr/sbin:/usr/X11R6/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

ulimit -c unlimited

