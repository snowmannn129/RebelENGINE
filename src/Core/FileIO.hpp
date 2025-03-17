#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <shared_mutex>
#include <future>
#include <cstdint>

namespace rebel_cad::core {

// Forward declarations
class MemoryPool;
}

namespace rebel_cad::core {

/**
 * @brief File I/O system for RebelCAD file format (.rebelcad)
 * 
 * Handles reading and writing of .rebelcad files with support for:
 * - Memory-mapped file access
 * - Streaming operations
 * - Compression
 * - Version control
 * - Error recovery
 * - Thread safety
 */
class FileIO {
public:
    // File format version
    struct Version {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
        uint8_t reserved;  // For alignment
    };

    // File flags
    enum class Flags : uint32_t {
        None = 0,
        Compressed = 1 << 0,
        Encrypted = 1 << 1,
        MemoryMapped = 1 << 2,
        DeltaEncoded = 1 << 3
    };

    // File header (56 bytes total)
    struct Header {
        char magic[8];           // "REBELCAD"
        Version version;         // 4 bytes
        Flags flags;            // 4 bytes
        uint64_t timestamp;     // 8 bytes
        uint8_t checksum[32];   // SHA-256 hash
    };

    // Section types
    enum class SectionType : uint32_t {
        Metadata,
        History,
        SceneGraph,
        Geometry,
        Sketch,
        FeatureTree,
        Assembly,
        Resources
    };

    // Section header
    struct SectionHeader {
        SectionType type;
        uint64_t offset;
        uint64_t size;
        uint64_t compressed_size;
        uint32_t crc32;
    };

    /**
     * @brief Construct a new FileIO object
     * @param memory_pool Shared pointer to memory pool for allocations
     * @param event_system Shared pointer to event system for notifications
     */
    FileIO(std::shared_ptr<MemoryPool> memory_pool, 
           std::shared_ptr<EventSystem> event_system);

    /**
     * @brief Create a new .rebelcad file
     * @param path File path
     * @param flags Initial file flags
     * @return true if successful
     */
    bool createFile(const std::filesystem::path& path, Flags flags = Flags::None);

    /**
     * @brief Open an existing .rebelcad file
     * @param path File path
     * @param read_only Whether to open in read-only mode
     * @return true if successful
     */
    bool openFile(const std::filesystem::path& path, bool read_only = false);

    /**
     * @brief Close the currently open file
     */
    void closeFile();

    /**
     * @brief Validate file header and checksum
     * @return true if valid
     */
    bool validateFile();

    /**
     * @brief Begin a write transaction
     * @return Transaction ID
     */
    uint64_t beginTransaction();

    /**
     * @brief Commit a write transaction
     * @param transaction_id Transaction to commit
     * @return true if successful
     */
    bool commitTransaction(uint64_t transaction_id);

    /**
     * @brief Rollback a write transaction
     * @param transaction_id Transaction to rollback
     */
    void rollbackTransaction(uint64_t transaction_id);

    /**
     * @brief Write data to a section
     * @param type Section type
     * @param data Data to write
     * @param compress Whether to compress the data
     * @return true if successful
     */
    bool writeSection(SectionType type, const std::vector<uint8_t>& data, 
                     bool compress = true);

    /**
     * @brief Read data from a section
     * @param type Section type
     * @param data Buffer to read into
     * @return true if successful
     */
    bool readSection(SectionType type, std::vector<uint8_t>& data);

    /**
     * @brief Get section information
     * @param type Section type
     * @return Section header
     */
    SectionHeader getSectionInfo(SectionType type) const;

    /**
     * @brief Check if file has changed externally
     * @return true if file was modified externally
     */
    bool checkExternalModification() const;

    /**
     * @brief Get current file version
     * @return Version struct
     */
    Version getVersion() const;

    /**
     * @brief Get current file flags
     * @return Flags
     */
    Flags getFlags() const;

private:
    // File handling
    std::filesystem::path m_file_path;
    std::unique_ptr<Header> m_header;
    std::vector<SectionHeader> m_sections;
    bool m_is_open;
    bool m_is_read_only;

    // Memory mapping
    void* m_mapped_memory;
    size_t m_mapped_size;

    // Dependencies
    std::shared_ptr<MemoryPool> m_memory_pool;
    std::shared_ptr<EventSystem> m_event_system;

    // Thread safety
    mutable std::shared_mutex m_file_mutex;

    // Transaction management
    struct Transaction {
        uint64_t id;
        std::vector<std::pair<SectionType, std::vector<uint8_t>>> changes;
        std::vector<uint8_t> original_state;
    };
    std::vector<Transaction> m_pending_transactions;
    uint64_t m_next_transaction_id;

    // Internal helpers
    bool writeHeader();
    bool readHeader();
    bool validateChecksum() const;
    void updateChecksum();
    bool mapFile();
    void unmapFile();
    std::vector<uint8_t> compressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data);
    void notifyFileChanged(SectionType type);
};

} // namespace rebel_cad::core
