# JPA reader

Will no longer be update here go to the actual repo
Almost all of jpcToJson.cpp actual working is credit to Jasper at [Noclip.website's github](https://github.com/magcius/noclip.website/blob/d7dd9e7212792ad4454cd00d2bdabc2ce78fea07/src/Common/JSYSTEM/JPA.ts#L4983). Used with permission with the goal of just editing textures in the end.
This is not to be used as a complete file reader, the goal will just to change some colors and possible textures. Provide your own jpc file as this repo will not distribute any of the games files.

use: `g++ jpcToJson.cpp -o toJson`

then: `toJson -i [jpc File]`

## Skyward Sword Param Parser

contains my *really* unoptimized and rough code for simple parameter parsing based on json stage outputs xD

The JSON library used is the nlohmann json library. This can be found on [This Github Page](https://github.com/nlohmann/json)

The json files read are produced by the [Skyward Sword Tools Repo](https://github.com/lepelog/skywardsword-tools) (the output path to read the stage files is a hard coded thing xD - like i said, rough)

## Building

I think due to the use of Filesystem, it requires at least c++ 17.
I compiled using g++

example: `g++ convertFlag.cpp` as converFlag.cpp is the main file

## Usage

`[compiled binary] ALL -n [ObjName] -f [bitfied name] -m [Param Mask] -s [Param Shift] -t [type option (see below)]`

`ALL` will use the stage file path to scan json files in

- Param mask is parsed as Hex `-m 0xFF` or `-m FF` is expected formatting
- the shift is parsed as decimal `-s 0` or `-s 20` is expected formatting
- ObjNames just reference the 'name' field in the output - case sensitive

type options are:

- `scene` for scene flag
- `story` for story flag (JP last 3 Hex digits)
- `item`  for item types (not full list yet)
- `point` if this is a specification for a point, it will grab the postion of it. Not fully supported with shifted indecies based on PATH
- `custom` This is used to match values to see patterns. `-t custom 4 [name_0] [name_1] [name_2] [name_3]` This is not smart processing, but it works for a tool xD
Adding a sort value can be made. this is to make quick refernce to same typed things that can be looked at on the object map for helping find use
`-sort [value]`

This can also be used to parse generic HEX strings. Pass in `-i [Hex String]`. NOTE: `ALL` command overwrites this.

example: `-i 0xFABFFFFF -m 0xFF -s 20` will have an ouput of `0xAB`
