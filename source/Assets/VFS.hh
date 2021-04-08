#pragma once

#include <functional>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <memory>
#include <utility>
#include <cmath>
#include <cassert>
#include <fstream>
#include <sys/stat.h>
#include <cstring>

namespace Assets {

#define CLASS_PTR(_class) typedef std::shared_ptr<class _class> _class##Ptr;\
						  typedef std::weak_ptr<class _class> _class##Weak;

#if VFS_LOGS_ENABLED
#   define VFS_LOG(...) printf(__VA_ARGS__)
#else
#   define VFS_LOG(...)
#endif

class CFileInfo final
{
public:
	CFileInfo();
	~CFileInfo();

	CFileInfo(const std::string& filePath);
	CFileInfo(const std::string& basePath, const std::string& fileName, bool isDir);

	void Initialize(const std::string& basePath, const std::string& fileName, bool isDir);
	const std::string& Name() const;
	const std::string& BaseName() const;
	const std::string& Extension() const;
	const std::string& AbsolutePath() const;
	const std::string& BasePath() const;

	bool IsDir() const;
	bool IsValid() const;

private:
	std::string m_Name;
	std::string m_BaseName;
	std::string m_Extension;
	std::string m_AbsolutePath;
	std::string m_BasePath;
	bool m_IsDir;
};

inline bool operator ==(const CFileInfo& fi1, const CFileInfo& fi2)
{
	return fi1.AbsolutePath() == fi2.AbsolutePath();
}

inline bool operator <(const CFileInfo& fi1, const CFileInfo& fi2)
{
	return fi1.AbsolutePath() < fi2.AbsolutePath();
}

class IFile
{
public:
	enum Origin
	{
		Begin,
		End,
		Set
	};

	enum FileMode
	{
		In = 0x01,
		Out = 0x02,
		ReadWrite = In | Out,
		Append = 0x04,
		Truncate = 0x08
	};

public:
	IFile() = default;
	~IFile() = default;

	virtual const CFileInfo& FileInfo() const = 0;
	virtual uint64_t Size() = 0;

	virtual bool IsReadOnly() const = 0;
	virtual void Open(int mode) = 0;
	virtual void Close() = 0;
	virtual bool IsOpened() const = 0;

	virtual uint64_t Seek(uint64_t offset, Origin origin) = 0;
	virtual uint64_t Tell() = 0;

	virtual uint64_t Read(uint8_t* buffer, uint64_t size) = 0;

	template<typename T>
	bool Read(T& value)
	{
		return (Read(&value, sizeof(value)) == sizeof(value));
	}
};

inline bool operator ==(const IFile& f1, const IFile& f2)
{
	return f1.FileInfo() == f2.FileInfo();
}

inline bool operator ==(std::shared_ptr<IFile> f1, std::shared_ptr<IFile> f2)
{
	if (!f1 || !f2)
	{
		return false;
	}

	return f1->FileInfo() == f2->FileInfo();
}

CLASS_PTR(IFile)

class IFileSystem
{
public:
	typedef std::set<IFilePtr> TFileList;

public:
	IFileSystem() = default;
	~IFileSystem() = default;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual bool IsInitialized() const = 0;
	virtual const std::string& BasePath() const = 0;
	virtual const TFileList& FileList() const = 0;
	virtual bool IsReadOnly() const = 0;
	virtual IFilePtr OpenFile(const CFileInfo& filePath, int mode) = 0;
	virtual void CloseFile(IFilePtr file) = 0;
	virtual bool IsFileExists(const CFileInfo& filePath) const = 0;
	virtual bool IsFile(const CFileInfo& filePath) const = 0;
	virtual bool IsDir(const CFileInfo& dirPath) const = 0;
};

class CStringUtils
{
public:
	static void Split(std::vector<std::string>& tokens, const std::string& text, char delimeter);
	static std::string Replace(std::string string, const std::string& search, const std::string& replace);

	static bool EndsWith(const std::string& fullString, const std::string& ending);
	static bool StartsWith(const std::string& fullString, const std::string& starting);
};

CLASS_PTR(IFile)
CLASS_PTR(IFileSystem)
CLASS_PTR(CVirtualFileSystem)

extern void vfs_initialize();
extern void vfs_shutdown();
extern CVirtualFileSystemPtr vfs_get_global();

class CVirtualFileSystem final
{
public:
	typedef std::list<IFileSystemPtr> TFileSystemList;
	typedef std::unordered_map<std::string, IFileSystemPtr> TFileSystemMap;

	struct SSortedAlias
	{
		std::string alias;
		IFileSystemPtr filesystem;

		SSortedAlias(const std::string& a,
					 IFileSystemPtr fs)
		: alias(a)
		, filesystem(fs)
		{}
	};
	typedef std::list<SSortedAlias> TSortedAliasList;

public:
	CVirtualFileSystem();
	~CVirtualFileSystem();

	void AddFileSystem(const std::string& alias, IFileSystemPtr filesystem);
	void RemoveFileSystem(const std::string& alias);
	bool IsFileSystemExists(const std::string& alias) const;
	IFileSystemPtr GetFilesystem(const std::string& alias);
	IFilePtr OpenFile(const CFileInfo& filePath, IFile::FileMode mode);
	void CloseFile(IFilePtr file);

private:
	TFileSystemMap m_FileSystem;
	TSortedAliasList m_SortedAlias;
	std::unordered_map<uintptr_t, IFileSystemPtr> m_OpenedFiles;
};

CLASS_PTR(CZip)

class CZip
{
public:
	CZip(const std::string& zipPath);
	~CZip();

	bool MapFile(const std::string& filename, std::vector<uint8_t>& data);
	const std::string& FileName() const;

	bool IsReadOnly() const;

private:
	std::string m_FileName;
	void* m_ZipArchive;
	typedef std::map<std::string, std::tuple<uint32_t, uint64_t>> TEntriesMap;
	static TEntriesMap s_Entries;
};


class CZipFile final : public IFile
{
	friend class CZipFileSystem;
public:
	CZipFile(const CFileInfo& fileInfo, CZipPtr zipFile);
	~CZipFile();

	virtual const CFileInfo& FileInfo() const override;
	virtual uint64_t Size() override;

	virtual bool IsReadOnly() const override;
	virtual void Open(int mode) override;
	virtual void Close() override;
	virtual bool IsOpened() const override;
	virtual uint64_t Seek(uint64_t offset, Origin origin) override;
	virtual uint64_t Tell() override;
	virtual uint64_t Read(uint8_t* buffer, uint64_t size) override;

private:
	CZipPtr m_ZipArchive;
	std::vector<uint8_t> m_Data;
	CFileInfo m_FileInfo;
	bool m_IsReadOnly;
	bool m_IsOpened;
	bool m_HasChanges;
	uint64_t m_SeekPos;
	int m_Mode;
};

CLASS_PTR(CZip)
CLASS_PTR(CZipFile)

class CZipFileSystem final : public IFileSystem
{
public:
	CZipFileSystem(const std::string& zipPath, const std::string& basePath);
	~CZipFileSystem();

	virtual void Initialize() override;
	virtual void Shutdown() override;
	virtual bool IsInitialized() const override;
	virtual const std::string& BasePath() const override;
	virtual const TFileList& FileList() const override;
	virtual bool IsReadOnly() const override;
	virtual IFilePtr OpenFile(const CFileInfo& filePath, int mode) override;
	virtual void CloseFile(IFilePtr file) override;
	virtual bool IsFileExists(const CFileInfo& filePath) const override;
	virtual bool IsFile(const CFileInfo& filePath) const override;
	virtual bool IsDir(const CFileInfo& dirPath) const override;

private:
	IFilePtr FindFile(const CFileInfo& fileInfo) const;

private:
	std::string m_ZipPath;
	CZipPtr m_Zip;
	std::string m_BasePath;
	bool m_IsInitialized;
	TFileList m_FileList;

	std::mutex m_Mutex;
	static std::unordered_map<std::string, CZipPtr> s_OpenedZips;
};

}