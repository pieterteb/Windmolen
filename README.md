Currently, Windmolen is a completely functional UCI chess engine (no Chess960 support) written in C. It uses a basic alpha-beta search and pure material evaluation.

Usage:
```bash
git clone https://github.com/pieterteb/Windmolen.git
cd Windmolen/src
make
./windmolen
```

Windmolen can be challenged as a [Lichess bot](https://lichess.org/@/Windmolen_bot) (if it is online).


The aim of this engine is to become as strong as possible, whilst also keeping the code well documented and easy to follow. Various C23 features are used as I am not concerned with compatibility with older code/compilers. Furthermore, I also do not care about outdated/horrific operating systems (like 32-bit systems and Windows 🤮).
