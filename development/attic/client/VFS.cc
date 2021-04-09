#include "VFS.hh"

#include <dirent.h>

#include "miniz.h"

namespace Assets {
CFileInfo::CFileInfo() {}
CFileInfo::~CFileInfo() {}

CFileInfo::CFileInfo(const std::string& basePath, const std::string& fileName, bool isDir)
{
	Initialize(basePath, fileName, isDir);
}

CFileInfo::CFileInfo(const std::string& filePath)
{
	std::size_t found = filePath.rfind("/");
	if (found != std::string::npos)
	{
		const std::string basePath = filePath.substr(0, found + 1);
		std::string fileName;
		if (found != filePath.length())
		{
			fileName = filePath.substr(found + 1, filePath.length() - found - 1);
		}

		Initialize(basePath, fileName, false);
	}
}

void CFileInfo::Initialize(const std::string& basePath, const std::string& fileName, bool isDir)
{
	m_BasePath = basePath;
	m_Name = fileName;
	m_IsDir = isDir;

	if (!CStringUtils::EndsWith(m_BasePath, "/"))
	{
		m_BasePath += "/";
	}

	if (isDir && !CStringUtils::EndsWith(m_Name, "/"))
	{
		m_Name += "/";
	}

	if (CStringUtils::StartsWith(m_Name, "/"))
	{
		m_Name = m_Name.substr(1, m_Name.length() - 1);
	}

	m_AbsolutePath = m_BasePath + m_Name;

	size_t dotsNum = std::count(m_Name.begin(), m_Name.end(), '.');
	bool isDotOrDotDot = (dotsNum == m_Name.length() && isDir);

	if (!isDotOrDotDot)
	{
		std::size_t found = m_Name.rfind(".");
		if (found != std::string::npos)
		{
			m_BaseName = m_Name.substr(0, found);
			if (found < m_Name.length())
			{
				m_Extension = m_Name.substr(found, m_Name.length() - found);
			}
		}
	}
}

const std::string& CFileInfo::Name() const
{
	return m_Name;
}

const std::string& CFileInfo::BaseName() const
{
	return m_BaseName;
}

const std::string& CFileInfo::Extension() const
{
	return m_Extension;
}

const std::string& CFileInfo::AbsolutePath() const
{
	return m_AbsolutePath;
}

const std::string& CFileInfo::BasePath() const
{
	return m_BasePath;
}

bool CFileInfo::IsDir() const
{
	return m_IsDir;
}

bool CFileInfo::IsValid() const
{
	return !m_AbsolutePath.empty();
}

void CStringUtils::Split(std::vector<std::string>& tokens, const std::string& text, char delimeter)
{
	size_t start = 0;
	size_t end = 0;
	while ((end = text.find(delimeter, start)) != std::string::npos)
	{
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
}

std::string CStringUtils::Replace(std::string string, const std::string& from, const std::string& to)
{
	size_t pos = 0;
	while ((pos = string.find(from, pos)) != std::string::npos)
	{
		string.replace(pos, from.length(), to);
		pos += to.length();
	}
	return string;
}

bool CStringUtils::EndsWith(std::string const& fullString, std::string const& ending)
{
	if (fullString.length() >= ending.length())
	{
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}

	return false;
}

bool CStringUtils::StartsWith(std::string const& fullString, std::string const& starting)
{
	if (fullString.length() >= starting.length())
	{
		return (0 == fullString.compare(0, starting.length(), starting));
	}

	return false;
}

static CVirtualFileSystemPtr g_FS;


struct SAliasComparator
{
	bool operator()(const CVirtualFileSystem::SSortedAlias& a1, const CVirtualFileSystem::SSortedAlias& a2) const
	{
		return a1.alias.length() > a2.alias.length();
	}
};

void vfs_initialize()
{
	if (!g_FS)
	{
		g_FS.reset(new CVirtualFileSystem());
	}
}

void vfs_shutdown()
{
	g_FS = nullptr;
}

CVirtualFileSystemPtr vfs_get_global()
{
	return g_FS;
}

CVirtualFileSystem::CVirtualFileSystem()
{
}

CVirtualFileSystem::~CVirtualFileSystem()
{
	std::for_each(m_FileSystem.begin(), m_FileSystem.end(), [](const TFileSystemMap::value_type& fs)
	{
		fs.second->Shutdown();
	});
}

void CVirtualFileSystem::AddFileSystem(const std::string& alias, IFileSystemPtr filesystem)
{
	if (!filesystem)
	{
		return;
	}

	std::string a = alias;
	if (!CStringUtils::EndsWith(a, "/"))
	{
		a += "/";
	}

	TFileSystemMap::const_iterator it = m_FileSystem.find(a);
	if (it == m_FileSystem.end())
	{
		m_FileSystem[a] = filesystem;
		m_SortedAlias.push_back(SSortedAlias(a, filesystem));
		m_SortedAlias.sort(SAliasComparator());
	}
}

void CVirtualFileSystem::RemoveFileSystem(const std::string& alias)
{
	std::string a = alias;
	if (!CStringUtils::EndsWith(a, "/"))
	{
		a += "/";
	}

	TFileSystemMap::const_iterator it = m_FileSystem.find(a);
	if (it == m_FileSystem.end())
	{
		m_FileSystem.erase(it);
	}
}

bool CVirtualFileSystem::IsFileSystemExists(const std::string& alias) const
{
	return (m_FileSystem.find(alias) != m_FileSystem.end());
}

IFilePtr CVirtualFileSystem::OpenFile(const CFileInfo& filePath, IFile::FileMode mode)
{
	IFilePtr file = nullptr;
	std::all_of(m_SortedAlias.begin(), m_SortedAlias.end(), [&](const TSortedAliasList::value_type& fs)
	{
		const std::string& alias = fs.alias;
		IFileSystemPtr filesystem = fs.filesystem;
		if (CStringUtils::StartsWith(filePath.BasePath(), alias) && filePath.AbsolutePath().length() != alias.length())
		{
			file = filesystem->OpenFile(filePath, mode);
		}

		if (file)
		{
			uintptr_t addr = reinterpret_cast<uintptr_t>(static_cast<void*>(file.get()));
			m_OpenedFiles[addr] = filesystem;

			return false;
		}

		return true;
	});

	return file;
}

void CVirtualFileSystem::CloseFile(IFilePtr file)
{
	uintptr_t addr = reinterpret_cast<uintptr_t>(static_cast<void*>(file.get()));

	std::unordered_map<uintptr_t, IFileSystemPtr>::const_iterator it = m_OpenedFiles.find(addr);
	if (it != m_OpenedFiles.end())
	{
		it->second->CloseFile(file);
		m_OpenedFiles.erase(it);
	}
}

CZip::TEntriesMap CZip::s_Entries;

CZip::CZip(const std::string& zipPath)
: m_FileName(zipPath)
{
	m_ZipArchive = static_cast<mz_zip_archive*>(malloc(sizeof(struct mz_zip_archive_tag)));
	memset(m_ZipArchive, 0, sizeof(struct mz_zip_archive_tag));

	mz_bool status = mz_zip_reader_init_file((mz_zip_archive*)m_ZipArchive, zipPath.c_str(), 0);
	if (!status)
	{
		VFS_LOG("Cannot open zip file: %s\n", zipPath.c_str());
		assert("Cannot open zip file" && false);
	}

	for (mz_uint i = 0; i < mz_zip_reader_get_num_files((mz_zip_archive*)m_ZipArchive); i++)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat((mz_zip_archive*)m_ZipArchive, i, &file_stat))
		{
			VFS_LOG("Cannot read entry with index: %d from zip archive", i, zipPath.c_str());
			continue;
		}

		s_Entries[file_stat.m_filename] = std::make_tuple(file_stat.m_file_index, file_stat.m_uncomp_size);
	}
}

CZip::~CZip()
{
	free(m_ZipArchive);
}

const std::string& CZip::FileName() const
{
	return m_FileName;
}

bool CZip::MapFile(const std::string &filename, std::vector<uint8_t>& data)
{
	TEntriesMap::const_iterator it = s_Entries.find(filename);
	if (it == s_Entries.end()) {
		return false;
	}

	uint32_t index = std::get<0>(it->second);
	uint64_t size = std::get<1>(it->second);
	data.resize((size_t)size);

	bool ok = mz_zip_reader_extract_to_mem_no_alloc((mz_zip_archive*)m_ZipArchive,
													index,
													data.data(),
													(size_t)size,
													0, 0, 0);
	return ok;
}

bool CZip::IsReadOnly() const
{
	struct stat fileStat;
	if (stat(FileName().c_str(), &fileStat) < 0) {
		return false;
	}
	return (fileStat.st_mode & S_IWUSR);
}

CZipFile::CZipFile(const CFileInfo& fileInfo, CZipPtr zipFile)
: m_ZipArchive(zipFile)
, m_FileInfo(fileInfo)
, m_IsReadOnly(true)
, m_IsOpened(false)
, m_HasChanges(false)
, m_SeekPos(0)
, m_Mode(0)
{
	assert(m_ZipArchive && "Cannot init zip file from empty zip archive");
}

CZipFile::~CZipFile() {}

const CFileInfo& CZipFile::FileInfo() const
{
	return m_FileInfo;
}

uint64_t CZipFile::Size()
{
	if (IsOpened())
	{
		return m_Data.size();
	}

	return 0;
}

bool CZipFile::IsReadOnly() const
{
	assert(m_ZipArchive && "Zip archive is epty");
	return (m_ZipArchive && m_ZipArchive->IsReadOnly() && m_IsReadOnly);
}

void CZipFile::Open(int mode)
{
	if ((mode & IFile::Out) ||
		(mode & IFile::Append)) {
		VFS_LOG("Files from zip can be opened in read only mode");
		return;
	}

	if (!FileInfo().IsValid() ||
		(IsOpened() && m_Mode == mode) ||
		!m_ZipArchive)
	{
		return;
	}

	std::string absPath = FileInfo().AbsolutePath();
	if (CStringUtils::StartsWith(absPath, "/"))
	{
		absPath = absPath.substr(1, absPath.length() - 1);
	}

	bool ok = m_ZipArchive->MapFile(absPath, m_Data);
	if (!ok) {
		VFS_LOG("Cannot open file: %s from zip: %s", absPath.c_str(), m_ZipArchive->Filename().c_str());
		return;
	}

	m_Mode = mode;
	m_IsReadOnly = true;
	m_SeekPos = 0;
	if (mode & IFile::Out)
	{
		m_IsReadOnly = false;
	}
	if (mode & IFile::Append)
	{
		m_IsReadOnly = false;
		m_SeekPos = Size() > 0 ? Size() - 1 : 0;
	}
	if (mode & IFile::Truncate)
	{
		m_Data.clear();
	}

	m_IsOpened = true;
}

void CZipFile::Close()
{
	if (IsReadOnly() || !m_HasChanges)
	{
		m_IsOpened = false;
		return;
	}


	m_IsOpened = false;
}

bool CZipFile::IsOpened() const
{
	return m_IsOpened;
}

uint64_t CZipFile::Seek(uint64_t offset, Origin origin)
{
	if (!IsOpened())
	{
		return 0;
	}

	if (origin == IFile::Begin)
	{
		m_SeekPos = offset;
	}
	else if (origin == IFile::End)
	{
		m_SeekPos = Size() - offset;
	}
	else
	{
		m_SeekPos += offset;
	}
	m_SeekPos = std::min(m_SeekPos, Size() - 1);

	return Tell();
}

uint64_t CZipFile::Tell()
{
	return m_SeekPos;
}

uint64_t CZipFile::Read(uint8_t* buffer, uint64_t size)
{
	if (!IsOpened())
	{
		return 0;
	}

	uint64_t bufferSize = Size() - Tell();
	uint64_t maxSize = std::min(size, bufferSize);
	if (maxSize > 0)
	{
		memcpy(buffer, m_Data.data(), (size_t)maxSize);
	}
	else
	{
		return 0;
	}

	return maxSize;
}

std::unordered_map<std::string, CZipPtr> CZipFileSystem::s_OpenedZips;

CZipFileSystem::CZipFileSystem(const std::string& zipPath, const std::string& basePath)
: m_ZipPath(zipPath)
, m_BasePath(basePath)
, m_IsInitialized(false)
{
}

CZipFileSystem::~CZipFileSystem()
{

}

void CZipFileSystem::Initialize()
{
	if (m_IsInitialized)
	{
		return;
	}

	std::lock_guard<decltype(m_Mutex)> lock(m_Mutex);
	m_Zip = s_OpenedZips[m_ZipPath];
	if (!m_Zip) {
		m_Zip.reset(new CZip(m_ZipPath));
		s_OpenedZips[m_ZipPath] = m_Zip;
	}
	m_IsInitialized = true;
}

void CZipFileSystem::Shutdown()
{
	std::lock_guard<decltype(m_Mutex)> lock(m_Mutex);
	m_Zip = nullptr;
	if (s_OpenedZips[m_ZipPath].use_count() == 1) {
		s_OpenedZips.erase(m_ZipPath);
	}
	m_FileList.clear();
	m_IsInitialized = false;
}

bool CZipFileSystem::IsInitialized() const
{
	return m_IsInitialized;
}

const std::string& CZipFileSystem::BasePath() const
{
	return m_BasePath;
}

const IFileSystem::TFileList& CZipFileSystem::FileList() const
{
	return m_FileList;
}

bool CZipFileSystem::IsReadOnly() const
{
	if (!IsInitialized())
	{
		return true;
	}

	return m_Zip->IsReadOnly();
}

IFilePtr CZipFileSystem::OpenFile(const CFileInfo& filePath, int mode)
{
	CFileInfo fileInfo(BasePath(), filePath.AbsolutePath(), false);
	IFilePtr file = FindFile(fileInfo);
	bool isExists = (file != nullptr);
	if (!isExists)
	{
		file.reset(new CZipFile(fileInfo, m_Zip));
	}
	file->Open(mode);

	if (!isExists && file->IsOpened())
	{
		m_FileList.insert(file);
	}

	return file;
}

void CZipFileSystem::CloseFile(IFilePtr file)
{
	if (file)
	{
		file->Close();
	}
}

bool CZipFileSystem::IsFileExists(const CFileInfo& filePath) const
{
	return (FindFile(BasePath() + filePath.AbsolutePath()) != nullptr);
}

bool CZipFileSystem::IsFile(const CFileInfo& filePath) const
{
	IFilePtr file = FindFile(filePath);
	if (file)
	{
		return !file->FileInfo().IsDir();
	}

	return false;
}

bool CZipFileSystem::IsDir(const CFileInfo& dirPath) const
{
	IFilePtr file = FindFile(dirPath);
	if (file)
	{
		return file->FileInfo().IsDir();
	}

	return false;
}

IFilePtr CZipFileSystem::FindFile(const CFileInfo& fileInfo) const
{
	TFileList::const_iterator it = std::find_if(m_FileList.begin(), m_FileList.end(), [&](IFilePtr file)
	{
		return file->FileInfo() == fileInfo;
	});

	if (it != m_FileList.end())
	{
		return *it;
	}

	return nullptr;
}

}