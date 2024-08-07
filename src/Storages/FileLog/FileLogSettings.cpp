#include <Parsers/ASTCreateQuery.h>
#include <Parsers/ASTFunction.h>
#include <Parsers/ASTSetQuery.h>
#include <Storages/FileLog/FileLogSettings.h>
#include <Common/Exception.h>
#include <Core/BaseSettings.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int UNKNOWN_SETTING;
    extern const int INVALID_SETTING_VALUE;
}

IMPLEMENT_SETTINGS_TRAITS(FileLogSettingsTraits, LIST_OF_FILELOG_SETTINGS)

void FileLogSettings::loadFromQuery(ASTStorage & storage_def)
{
    if (storage_def.settings)
    {
        try
        {
            applyChanges(storage_def.settings->changes);
        }
        catch (Exception & e)
        {
            if (e.code() == ErrorCodes::UNKNOWN_SETTING)
                e.addMessage("for storage " + storage_def.engine->name);
            throw;
        }
    }
    else
    {
        auto settings_ast = std::make_shared<ASTSetQuery>();
        settings_ast->is_standalone = false;
        storage_def.set(storage_def.settings, settings_ast);
    }

    /// Check that batch size is not too high (the same as we check setting max_block_size).
    constexpr UInt64 max_sane_block_rows_size = 4294967296; // 2^32
    if (poll_max_batch_size > max_sane_block_rows_size)
        throw Exception(ErrorCodes::INVALID_SETTING_VALUE, "Sanity check: 'poll_max_batch_size' value is too high ({})", poll_max_batch_size);
}

}
