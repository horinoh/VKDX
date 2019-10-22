#ifdef USE_PIPELINE_SERIALIZE
class PipelineCacheSerializer
{
public:
	PipelineCacheSerializer(VkDevice Dev, const std::wstring& Path, const size_t Count) : Device(Dev), FilePath(Path) {
#ifdef ALWAYS_REBUILD_PIPELINE
		DeleteFile(FilePath.c_str());
#endif
		//!< ファイルが読めた場合は PipelineCaches[0] へ読み込む (If file is read, load to PipelineCaches[0])
		std::ifstream In(FilePath.c_str(), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			Log("Reading PipelineCache\n");
			In.seekg(0, std::ios_base::end);
			const size_t Size = In.tellg();
			In.seekg(0, std::ios_base::beg);
			if (Size) {
				auto Data = new char[Size];
				In.read(Data, Size);
				Validate(Size, Data);
				PipelineCaches.resize(1); //!< 読み込めた場合は1つで良い(書き込む際にマージされているはず)
				const VkPipelineCacheCreateInfo PCCI = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, nullptr, 0, Size, Data };
				VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, nullptr, PipelineCaches.data()));
				delete[] Data;
				IsLoaded = true;
			}
			In.close();
		}
		//!< ファイルが読めなかった場合は書き込み用のパイプラインキャッシュを(必要に応じて複数個)作成する (If file is not read, create PiplineCache array for write)
		if (!IsLoaded) {
			Log("Creating PipelineCache\n");
			PipelineCaches.resize(Count); //!< 書き込む場合は複数必要な可能性がある
			const VkPipelineCacheCreateInfo PCCI = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, nullptr, 0, 0, nullptr };
			for (auto& i : PipelineCaches) {
				VERIFY_SUCCEEDED(vkCreatePipelineCache(Device, &PCCI, nullptr, &i));
			}
		}
	}
	virtual ~PipelineCacheSerializer() {
		if (!IsLoaded) {
			Log("Writing PipelineCache\n");
			//!< パイプラインキャッシュが複数ある場合、末尾のパイプラインキャッシュへマージする (Merge PipelineCaches to the last element)
			if (PipelineCaches.size() > 1) {
				VERIFY_SUCCEEDED(vkMergePipelineCaches(Device, PipelineCaches.back(), static_cast<uint32_t>(PipelineCaches.size() - 1), PipelineCaches.data()));
			}
			//!< 末尾のパイプラインキャッシュをファイルへ書き込む (Write last element of PipelineCache to file)
			size_t Size;
			VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCaches.back(), &Size, nullptr));
			if (Size) {
				auto Data = new char[Size];
				VERIFY_SUCCEEDED(vkGetPipelineCacheData(Device, PipelineCaches.back(), &Size, Data));
				std::ofstream Out(FilePath.c_str(), std::ios::out | std::ios::binary);
				if (!Out.fail()) {
					Out.write(Data, Size);
					Out.close();
				}
				delete[] Data;
			}
		}
		for (auto i : PipelineCaches) {
			vkDestroyPipelineCache(Device, i, nullptr);
		}
		PipelineCaches.clear();
		LOG_OK();
	}
	void Validate(const size_t Size, const void* Data) {
		auto Ptr = reinterpret_cast<const uint32_t*>(Data);
		const auto PCSize = *Ptr++;
		const auto PCVersion = *Ptr++;
		const auto PCVenderID = *Ptr++;
		const auto PCDeviceID = *Ptr++;
		uint8_t PCUUID[VK_UUID_SIZE];
		memcpy(PCUUID, Ptr, sizeof(PCUUID));

		assert(Size == PCSize && "");
		assert(VK_PIPELINE_CACHE_HEADER_VERSION_ONE == PCVersion && "");

		Log("PipelineCacheSerializer\n");
		Logf("\tSize = %d\n", Size);
		Logf("\tVersion = %s\n", PCVersion == VK_PIPELINE_CACHE_HEADER_VERSION_ONE ? "VK_PIPELINE_CACHE_HEADER_VERSION_ONE" : "Unknown");
		Logf("\tVenderID = %d\n", PCVenderID);
		Logf("\tDeviceID = %d\n", PCDeviceID);
		Log("\tUUID = "); for (auto i = 0; i < sizeof(PCUUID); ++i) { Logf("%c", PCUUID[i]); } Log("\n");
	}
	VkPipelineCache GetPipelineCache(const size_t i) const { return IsLoaded ? PipelineCaches[0] : PipelineCaches[i]; }
private:
	VkDevice Device;
	std::wstring FilePath;
	std::vector<VkPipelineCache> PipelineCaches;
	bool IsLoaded = false;
};
#endif //!< USE_PIPELINE_SERIALIZE