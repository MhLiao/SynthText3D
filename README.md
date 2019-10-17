# SynthText3D: Synthesizing Scene Text Images from 3D Virtual Worlds

## Introduction
This is a project that synthesizes scene text images from 3D virtual worlds. 

For more details, please refer to our [arXiv paper](https://arxiv.org/abs/1907.06007). 

![Pipeline](./imgs/pipeline.jpg)

## Performance
Detection results with different synthetic data. ``5k", ``10k'' and ``800k" indicate the number of images used for training.

|         Training data         | ICDAR 2015 |      |      | ICDAR 2013 |      |      |  MLT |      |      |
|:-----------------------------:|:----------:|:----:|:----:|:----------:|:----:|:----:|:----:|:----:|:----:|
|                               |      P     |   R  |   F  |      P     |   R  |   F  |   P  |   R  |   F  |
|         SynthText 10k         |    40.1    | 54.8 | 46.3 |    54.5    | 69.4 | 61.1 | 34.3 | 41.4 | 37.5 |
|         SynthText 800k        |    67.1    | 51.0 | 57.9 |    68.9    | 66.4 | 67.7 | 53.9 | 36.5 | 43.5 |
|            VISD 10k           |    73.3    | 59.5 | 65.7 |    73.2    | 68.5 | 70.8 | 58.9 | 40.0 | 47.6 |
|      Ours 10k (10 scenes)     |    64.5    | 56.7 | 60.3 |    75.8    | 65.6 | 70.4 | 50.4 | 39.0 | 44.0 |
|      Ours 10k (20 scenes)     |    69.8    | 58.1 | 63.4 |    76.6    | 66.0 | 70.9 | 51.3 | 41.1 | 45.6 |
|      Ours 10k (30 scenes)     |    71.2    | 62.1 | 66.3 |    77.1    | 67.3 | 71.9 | 55.4 | 43.3 | 48.6 |
| Ours 5k (10 scenes) + VISD 5k |    71.1    | 64.4 | 67.6 |    76.5    | 71.4 | 73.8 | 57.6 | 44.2 | 49.8 |

## Video Demo
We made a video demonstration for the synthesis process and visualization. Click the following link to watch the video on [YouTube](https://youtu.be/hI6HfuEkcEw). 

![Sample imgs](./imgs/demo.jpg)
  
## Data
For our 10K data set synthesized from 30 scenes, download from [Google Drive](https://drive.google.com/open?id=1MIkffNmgPu1kgP6kI-5yJ43sCUki9ORg)

### Data Formats
The extracted data folder has the following format:

```
|- Synth3D-10K
|    |- label
|    |    |- 1.txt
|    |    |- 2.txt
|    |    |- 3.txt
|    |    |- ...
|    |    |- 10000.txt
|    |- img
|    |    |- 1.jpg
|    |    |- 2.jpg
|    |    |- 3.jpg
|    |    |- ...
|    |    |- 10000.jpg

```

Each text instance is the label files takes up 5 lines:

```
x1,y1
x2,y2
x3,y3
x4,y4
is_difficult
```

when `is_difficult==1`, the text is marked as difficult. The coordinates are arranged clockwise. 


## Code
See `./Code`. `./Code/Unrealtext-Source` is adapted from [UnrealCV](https://github.com/unrealcv/unrealcv) and implements functionalities for text synthesis. 

### How to use (Ubuntu)

#### Installation
1. Make sure you have UE4.16 installed and the UnrealCV plugin functions normally. 
2. Ask an artist to create a virtual scene or download one from the [Unreal Market](https://www.unrealengine.com/marketplace/ja/store). 
3. Use UE4.16 to compile the unrealtext source code and put the plugin into your unreal project. 
4. Open your unreal project, add the unrealtext plugin. Compile the `myCameraRecordPawn.h/cpp` class
5. Add the following components: PugTextPawn, myCameraRecordPawn. You need to add `n` PugTextPawn pawns to render `n` text instances in the scene. 
6. Package the environment

#### Data Generation
1. Set camera anchors: launch the executable, manually wander around the scene and use `mouse left click` to record anchors. The trajectory file is stored at `./{YourSceneRoot}/LinuxNoEditor/UnrealText/Binaries/Linux/trajectory.txt`. 
2. Run `python3 RetrieveSceneInfo.py` to obtain scene informations such as depth map and normal map for each camera anchor location. 
3. Run `python3 GenerateData.py` to generate data. 

## Citing the related works

Please cite the paper in your publications if it helps your research:

	@article{liao2019synthtext3d,
	  title={SynthText3D: Synthesizing Scene Text Images from 3D Virtual Worlds},
	  author={Liao, Minghui and Song, Boyu and He, Minghang and Long, Shangbang and Yao, Cong and Bai, Xiang},
	  journal={arXiv preprint arXiv:1907.06007},
	  year={2019}
	}
