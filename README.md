## Setup Instructions

### Install UnrealEngine

```
git clone -b 4.22 git@github.com:EpicGames/UnrealEngine.git
cd UnrealEngine
./Setup.sh
./GenerateProjectFiles.sh
make
```

For more info vist [Building on Linux](https://wiki.unrealengine.com/Building_On_Linux).

### Git Large File Storage (Git LFS)

```
sudo apt-get install git-lfs
git-lfs install
```

For more info visit [Git LFS](https://git-lfs.github.com/)

## Setup Synthetic (this repo)

Clone repository:

```
git clone git@github.com:VertexStudio/Synthetic.git
```

Environment variables:

```
export UE4_ROOT=/path/to/UE4.22
export UE4_SYNTHETIC=/path/to/Synthetic
```

Generate project files and build

```
cd $UE4_SYNTHETIC
git lfs pull
./generate.sh
./build.sh
```

Run:

```
./run_editor.sh
```
