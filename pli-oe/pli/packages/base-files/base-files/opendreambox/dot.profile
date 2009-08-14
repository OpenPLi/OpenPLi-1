# ~/.profile: executed by Bourne-compatible login shells.

if [ "$PS1" ]; then
# works for bash and ash (no other shells known to be in use here)
   PS1="\[\e[35;1m\]\u@\h \w #\[\e[0m\] "
fi

export PS1

alias m=less
alias ls='ls -CF'
alias l='ls -lF'
alias port='nc localhost'

if [ -f ~/.bashrc ]; then
  . ~/.bashrc
fi

# path set by /etc/profile
# export PATH

mesg n
