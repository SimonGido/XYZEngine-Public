#pragma once
#include "Asset.h"




namespace XYZ {
	class AssetSerializer
	{
	public:
		static void SerializeAsset(const Ref<Asset>& asset);
			
		static Ref<Asset> LoadAsset(Ref<Asset>& asset);
		
		static Ref<Asset> LoadAssetMeta(const std::string& filepath, const GUID& directoryHandle, AssetType type);

	private:
		template <typename T>
		static Ref<Asset> deserialize(const Ref<Asset>& asset);

		template <typename T>
		static void serialize(const Ref<Asset>& asset);

		
		static void loadMetaFile(Ref<Asset>& asset);
		static void createMetaFile(const Ref<Asset>& asset);
	};
}