﻿//#type compute
#version 460

struct DrawCommand 
{
	uint Count;         
	uint InstanceCount; 
	uint FirstIndex;    
	uint BaseVertex;    
	uint BaseInstance;  
};

struct ParticleData
{
	vec4  Color;
	vec4  TexCoord;
	vec2  Position;
	vec2  Size;
	float Rotation;

	float Alignment0;
	float Alignment1;
	float Alignment2;
};

struct ParticleSpecification
{
	vec4  StartColor;		  
	vec2  StartSize;		  
	vec2  StartVelocity;
	vec2  StartPosition;

	float TimeAlive;
	int   IsAlive;
};

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec4 u_ViewPosition;
};

layout(std430, binding = 1) buffer buffer_Data // render // particle
{
	ParticleData Data[];
};

layout(std430, binding = 2) buffer buffer_Specification // particle
{
	ParticleSpecification Specification[];
};

layout(std140, binding = 3) buffer buffer_DrawCommand // indirect
{
	DrawCommand Command;
};

layout(binding = 4, offset = 0) uniform atomic_uint counter_DeadParticles;

struct ColorOverLifeTime
{
	vec4 StartColor;
	vec4 EndColor;
};

void UpdateColorOverLifeTime(out vec4 result, vec4 color, ColorOverLifeTime colt, float ratio)
{
	result = color * mix(colt.StartColor, colt.EndColor, ratio);
}

struct SizeOverLifeTime
{
	vec2 StartSize;
	vec2 EndSize;
};

void UpdateSizeOverLifeTime(out vec2 result, vec2 size, SizeOverLifeTime solt, float ratio)
{
	result = size * mix(solt.StartSize, solt.EndSize, ratio);
}

struct TextureSheetAnimation
{
	int TilesX;
	int TilesY;
};

void UpdateTextureSheetOverLifeTime(out vec4 texCoord, float tilesX, float tilesY, float ratio)
{
	float stageCount = tilesX * tilesY;
	float stageProgress = ratio * stageCount;

	int index  = int(floor(stageProgress));
	int column = int(mod(index, tilesX));
	int row	   = int(index / tilesY);
	float sizeX = 1.0f / tilesX;
	float sizeY = 1.0f / tilesY;

	texCoord.xy = vec2(float(column) / tilesX, float(row) / tilesY);
	texCoord.zw = texCoord.xy + vec2(sizeX, sizeY);
}

struct Main
{
	int   Repeat;
	int   ParticlesEmitted;
	float Time;
	float Speed;
	float LifeTime;
};

void UpdatePosition(inout vec2 result, vec2 velocity, float speed, float ts)
{
	result += vec2(velocity.x * speed * ts, velocity.y * speed * ts);
}


void BuildDrawCommand(int particlesInExistence)
{
	uint deadCount = atomicCounter(counter_DeadParticles);
	Command.Count = 6;
	Command.InstanceCount = particlesInExistence - deadCount;
	Command.FirstIndex = 0;
	Command.BaseVertex = 0;
	Command.BaseInstance = deadCount;
}


uniform ColorOverLifeTime	  u_ColorModule;
uniform SizeOverLifeTime	  u_SizeModule;
uniform TextureSheetAnimation u_TextureModule;
uniform Main			      u_MainModule;
uniform vec2				  u_Force;
uniform int					  u_MaxParticles;

void KillParticle(inout float timeAlive, inout int isAlive, inout vec2 position, vec2 startPosition)
{	
	timeAlive = 0.0;
	isAlive = u_MainModule.Repeat;
	position = startPosition;
	if (u_MainModule.Repeat == 0)
		atomicCounterIncrement(counter_DeadParticles);
}


layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	uint deadCount = atomicCounter(counter_DeadParticles);
	if (id > u_MaxParticles || id < deadCount)
		return;

	ParticleData data			= Data[id];
	ParticleSpecification specs = Specification[id];
	specs.TimeAlive				+= u_MainModule.Time;
	if (specs.TimeAlive > u_MainModule.LifeTime)
		KillParticle(specs.TimeAlive, specs.IsAlive, data.Position, specs.StartPosition);

	float ratio   = specs.TimeAlive / u_MainModule.LifeTime;
	vec2 velocity = specs.StartVelocity + (u_Force * ratio);
	UpdateColorOverLifeTime(data.Color, specs.StartColor, u_ColorModule, ratio);
	UpdateSizeOverLifeTime(data.Size, specs.StartSize, u_SizeModule, ratio);
	UpdatePosition(data.Position, velocity, u_MainModule.Speed, u_MainModule.Time);
	UpdateTextureSheetOverLifeTime(data.TexCoord, u_TextureModule.TilesX, u_TextureModule.TilesY, ratio);
	
	BuildDrawCommand(u_MainModule.ParticlesEmitted);

	Data[id]		  = data;
	Specification[id] = specs;
}
