// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitformat.hpp"

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

#include <unordered_map>
#include <algorithm>
#include <cwctype>
#include <cstdint>

#include "7zip/IStream.h"

using namespace std;

namespace bit7z {
    namespace BitFormat {
        const BitInFormat        Auto( 0x00 );
        const BitInOutFormat      Zip( 0x01, L".zip", MULTIPLE_FILES | COMPRESSION_LEVEL | ENCRYPTION );
        const BitInOutFormat    BZip2( 0x02, L".bz2", COMPRESSION_LEVEL | INMEM_COMPRESSION );
        const BitInFormat         Rar( 0x03 );
        const BitInFormat         Arj( 0x04 );
        const BitInFormat           Z( 0x05 );
        const BitInFormat         Lzh( 0x06 );
        const BitInOutFormat SevenZip( 0x07, L".7z", MULTIPLE_FILES | SOLID_ARCHIVE | COMPRESSION_LEVEL |
                                       ENCRYPTION | HEADER_ENCRYPTION );
        const BitInFormat         Cab( 0x08 );
        const BitInFormat        Nsis( 0x09 );
        const BitInFormat        Lzma( 0x0A );
        const BitInFormat      Lzma86( 0x0B );
        const BitInOutFormat       Xz( 0x0C, L".xz",
                                       COMPRESSION_LEVEL | ENCRYPTION | HEADER_ENCRYPTION | INMEM_COMPRESSION );
        const BitInFormat        Ppmd( 0x0D );
        const BitInFormat        COFF( 0xC7 );
        const BitInFormat         Ext( 0xC7 );
        const BitInFormat        VMDK( 0xC8 );
        const BitInFormat         VDI( 0xC9 );
        const BitInFormat        QCow( 0xCA );
        const BitInFormat         GPT( 0xCB );
        const BitInFormat        Rar5( 0xCC );
        const BitInFormat        IHex( 0xCD );
        const BitInFormat         Hxs( 0xCE );
        const BitInFormat          TE( 0xCF );
        const BitInFormat       UEFIc( 0xD0 );
        const BitInFormat       UEFIs( 0xD1 );
        const BitInFormat    SquashFS( 0xD2 );
        const BitInFormat      CramFS( 0xD3 );
        const BitInFormat         APM( 0xD4 );
        const BitInFormat        Mslz( 0xD5 );
        const BitInFormat         Flv( 0xD6 );
        const BitInFormat         Swf( 0xD7 );
        const BitInFormat        Swfc( 0xD8 );
        const BitInFormat        Ntfs( 0xD9 );
        const BitInFormat         Fat( 0xDA );
        const BitInFormat         Mbr( 0xDB );
        const BitInFormat         Vhd( 0xDC );
        const BitInFormat          Pe( 0xDD );
        const BitInFormat         Elf( 0xDE );
        const BitInFormat       Macho( 0xDF );
        const BitInFormat         Udf( 0xE0 );
        const BitInFormat         Xar( 0xE1 );
        const BitInFormat         Mub( 0xE2 );
        const BitInFormat         Hfs( 0xE3 );
        const BitInFormat         Dmg( 0xE4 );
        const BitInFormat    Compound( 0xE5 );
        const BitInOutFormat      Wim( 0xE6, L".wim", MULTIPLE_FILES );
        const BitInFormat         Iso( 0xE7 );
        const BitInFormat         Chm( 0xE9 );
        const BitInFormat       Split( 0xEA );
        const BitInFormat         Rpm( 0xEB );
        const BitInFormat         Deb( 0xEC );
        const BitInFormat        Cpio( 0xED );
        const BitInOutFormat      Tar( 0xEE, L".tar", MULTIPLE_FILES | INMEM_COMPRESSION );
        const BitInOutFormat     GZip( 0xEF, L".gz", COMPRESSION_LEVEL | INMEM_COMPRESSION );

        const unordered_map< wstring, const BitInFormat& > common_extensions {
            { L"7z",       SevenZip },
            { L"bzip2",    BZip2 },
            { L"bz2",      BZip2 },
            { L"tbz2",     BZip2 },
            { L"tbz",      BZip2 },
            { L"gz",       GZip },
            { L"gzip",     GZip },
            { L"tgz",      GZip },
            { L"tar",      Tar },
            { L"wim",      Wim },
            { L"swm",      Wim },
            { L"xz",       Xz },
            { L"txz",      Xz },
            { L"zip",      Zip },
            { L"zipx",     Zip },
            { L"jar",      Zip },
            { L"xpi",      Zip },
            { L"odt",      Zip },
            { L"ods",      Zip },
            { L"odp",      Zip },
            { L"docx",     Zip },
            { L"xlsx",     Zip },
            { L"pptx",     Zip },
            { L"epub",     Zip },
            { L"001",      Split },
            { L"ar",       Deb },
            { L"apm",      APM },
            { L"arj",      Arj },
            { L"cab",      Cab },
            { L"chm",      Chm },
            { L"chi",      Chm },
            { L"msi",      Compound },
            { L"doc",      Compound },
            { L"xls",      Compound },
            { L"ppt",      Compound },
            { L"msg",      Compound },
            { L"cpio",     Cpio },
            { L"cramfs",   CramFS },
            { L"deb",      Deb },
            { L"dmg",      Dmg },
            { L"dll",      Pe },
            { L"dylib",    Macho },
            { L"exe",      Pe }, //note: we do not distinguish 7z SFX exe at the moment!
            { L"ext",      Ext },
            { L"ext2",     Ext },
            { L"ext3",     Ext },
            { L"ext4",     Ext },
            { L"fat",      Fat },
            { L"flv",      Flv },
            { L"hfs",      Hfs },
            { L"hfsx",     Hfs },
            { L"hxs",      Hxs },
            { L"ihex",     IHex },
            { L"lzh",      Lzh },
            { L"lha",      Lzh },
            { L"lzma",     Lzma },
            { L"lzma86",   Lzma86 },
            { L"mbr",      Mbr },
            { L"mslz",     Mslz },
            { L"mub",      Mub },
            { L"nsis",     Nsis },
            { L"ntfs",     Ntfs },
            { L"ppmd",     Ppmd },
            { L"qcow",     QCow },
            { L"qcow2",    QCow },
            { L"qcow2c",   QCow },
            { L"rpm",      Rpm },
            { L"scap",     UEFIc },
            { L"squashfs", SquashFS },
            { L"udf",      Udf },
            { L"uefif",    UEFIs },
            { L"vmdk",     VMDK },
            { L"vdi",      VDI },
            { L"vhd",      Vhd },
            { L"xar",      Xar },
            { L"pkg",      Xar },
            { L"z",        Z },
            { L"taz",      Z }
        };

        /* NOTE: For signatures with less than 8 bytes (size of uint64_t), remaining bytes are set to 0 */
        const unordered_map< uint64_t, const BitInFormat& > common_signatures = {
            { 0x526172211A070000, Rar },
            { 0x526172211A070100, Rar5 },
            { 0x4657530000000000, Swf },
            { 0x4357530000000000, Swfc },
            { 0x377ABCAF271C0000, SevenZip },
            { 0x425A680000000000, BZip2 },
            { 0x1F8B080000000000, GZip },
            { 0x4D5357494D000000, Wim },
            { 0xFD377A585A000000, Xz },
            { 0x504B000000000000, Zip },
            { 0x4552000000000000, APM },
            { 0x60EA000000000000, Arj },
            { 0x4D53434600000000, Cab },
            { 0x4954534600000000, Chm },
            { 0xD0CF11E0A1B11AE1, Compound },
            { 0xC771000000000000, Cpio },
            { 0x71C7000000000000, Cpio },
            { 0x3037303730000000, Cpio },
            { 0x213C617263683E00, Deb },
            //{ 0x7801730D62626000, Dmg }, /* DMG signature detection is not this simple */
            { 0x7F454C4600000000, Elf },
            { 0x4D5A000000000000, Pe },
            { 0x464C560100000000, Flv },
            { 0x5D00000000000000, Lzma },
            { 0x015D000000000000, Lzma86 },
            { 0xCFFAEDFE00000000, Macho },
            { 0xCAFEBABE00000000, Macho },
            { 0x535A444488F02733, Mslz },
            { 0x514649FB00000000, QCow },
            { 0xEDABEEDB00000000, Rpm },
            { 0x7371736800000000, SquashFS },
            { 0x6873717300000000, SquashFS },
            { 0x4B444D0000000000, VMDK },
            { 0x3C3C3C2000000000, VDI }, //Alternatively 0x7F10DABE at offset 0x40
            { 0x636F6E6563746978, Vhd },
            { 0x7861722100000000, Xar },
            { 0x1F9D000000000000, Z },
            { 0x1FA0000000000000, Z }
        };

        struct OffsetSignature {
            uint64_t signature;
            std::streamoff offset;
            uint32_t size;
            const BitInFormat& format;
        };

        const vector< OffsetSignature > common_signatures_with_offset = {
            { 0x2D6C680000000000, 0x02,  3, Lzh },
            { 0x7F10DABE00000000, 0x40,  4, VDI },
            { 0x7573746172000000, 0x101, 5, Tar },
            { 0x4244000000000000, 0x400, 2, Hfs },
            { 0x482B000000000000, 0x400, 2, Hfs },
            { 0x4858000000000000, 0x400, 2, Hfs }/*,
            { 0x4344303031, 0x8001, 5, Iso },
            { 0x4344303031, 0x8801, 5, Iso },
            { 0x4344303031, 0x9001, 5, Iso }*/
        };

        uint64_t read_signature( IInStream* stream, uint32_t size ) {
            uint64_t signature = 0;
            stream->Read( &signature, size, nullptr );
            return _byteswap_uint64( signature );
        }

        const BitInFormat& detectFormatFromSig( IInStream* stream ) {
            constexpr auto SIGNATURE_SIZE = 8u;

            uint64_t file_signature = read_signature( stream, SIGNATURE_SIZE );
            uint64_t signature_mask = 0xFFFFFFFFFFFFFFFFull;
            for ( auto i = 0u; i < SIGNATURE_SIZE - 1; ++i ) {
                auto it = common_signatures.find( file_signature );
                if ( it != common_signatures.end() ) {
                    //std::wcout << L"Detected format: " << std::hex << it->second.value() << std::endl;
                    stream->Seek( 0, 0, nullptr );
                    return it->second;
                }
                signature_mask <<= 8ull;          // left shifting the mask of 1 byte, so that
                file_signature &= signature_mask; // the least significant i bytes are masked (set to 0)
            }

            for ( auto& sig : common_signatures_with_offset ) {
                //stream.seekg( sig.offset );
                stream->Seek( sig.offset, 0, nullptr );
                file_signature = read_signature( stream, sig.size );
                if ( file_signature == sig.signature ) {
                    stream->Seek( 0, 0, nullptr );
                    return sig.format;
                }
            }

            // Detecting ISO/UDF
            constexpr auto ISO_SIGNATURE              = 0x4344303031000000; //CD001
            constexpr auto ISO_SIGNATURE_SIZE         = 5ull;
            constexpr auto ISO_SIGNATURE_OFFSET       = 0x8001;

            //Checking for ISO signature
            //stream.seekg( ISO_SIGNATURE_OFFSET );
            stream->Seek( ISO_SIGNATURE_OFFSET, 0, nullptr );
            file_signature = read_signature( stream, ISO_SIGNATURE_SIZE );
            if ( file_signature == ISO_SIGNATURE ) {
                constexpr auto MAX_VOLUME_DESCRIPTORS     = 16;
                constexpr auto ISO_VOLUME_DESCRIPTOR_SIZE = 0x800; //2048

                constexpr auto UDF_SIGNATURE          = 0x4E53523000000000; //NSR0
                constexpr auto UDF_SIGNATURE_SIZE     = 4u;

                //The file is ISO, checking if it is also UDF!
                for ( auto descriptor_index = 1ull; descriptor_index < MAX_VOLUME_DESCRIPTORS; ++descriptor_index ) {
                    //stream.seekg( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE );
                    stream->Seek( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE, 0, nullptr );
                    file_signature = read_signature( stream, UDF_SIGNATURE_SIZE );
                    if ( file_signature == UDF_SIGNATURE ) {
                        //std::wcout << "UDF!" << std::endl;
                        stream->Seek( 0, 0, nullptr );
                        return Udf;
                    }
                }
                //std::wcout << "ISO!" << std::endl;
                stream->Seek( 0, 0, nullptr );
                return Iso; //No UDF volume signature found, i.e. simple ISO!
            }

            stream->Seek( 0, 0, nullptr );
            throw BitException( "Cannot detect the format of the file" );
        }

        const BitInFormat& detectFormatFromExt( const wstring& in_file ) {
            wstring ext = filesystem::fsutil::extension( in_file );
            if ( ext.empty() ) {
                throw BitException( "Cannot detect the archive format from the extension" );
            }

            std::transform( ext.cbegin(), ext.cend(), ext.begin(), std::towlower );
            //std::wcout << ext << std::endl;

            //detecting archives with common file extensions
            auto it = common_extensions.find( ext );
            if ( it != common_extensions.end() ) { //extension found in map
                return it->second;
            }

            //detecting multivolume archives extension
            if ( ( ext[ 0 ] == L'r' || ext[ 0 ] == L'z' ) &&
                    ( ext.size() == 3 && iswdigit( ext[ 1 ] ) != 0 && iswdigit( ext[ 2 ] ) != 0 ) ) {
                //extension follows the format zXX or rXX, where X is a number in range [0-9]
                return ext[ 0 ] == L'r' ? Rar : Zip;
            }

            //TODO: 7z SFX detection
            /*if ( ext == L"exe" ) { //check properties to see if 7z SFX

            }*/

            //Note: iso, img and ima extensions can be associated with different formats -> detect by signature

            /* The extension did not match any known format extension, delegating the decision to the client */
            return Auto;
        }
    }
}

using namespace bit7z;

BitInFormat::BitInFormat( unsigned char value ) : mValue( value ) {}

int BitInFormat::value() const {
    return mValue;
}

bool BitInFormat::operator==( const BitInFormat& other ) const {
    return mValue == other.value();
}

bool BitInFormat::operator!=( const BitInFormat& other ) const {
    return !( *this == other );
}

const GUID BitInFormat::guid() const {
#if _MSC_VER <= 1700
    GUID ret;
    ret.Data1 = 0x23170F69;
    ret.Data2 = 0x40C1;
    ret.Data3 = 0x278A;

    const unsigned char data4 [] = { 0x10, 0x00, 0x00, 0x01, 0x10, mValue, 0x00, 0x00 };
    std::copy( data4, data4 + 8, ret.Data4 );

    return ret;
#else
    return { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, mValue, 0x00, 0x00 } };
#endif
}

BitInOutFormat::BitInOutFormat( unsigned char value, const wstring& ext, bitset< FEATURES_COUNT > features ) :
    BitInFormat( value ), mExtension( ext ), mFeatures( features ) {}

const wstring& BitInOutFormat::extension() const {
    return mExtension;
}

const bitset< FEATURES_COUNT > BitInOutFormat::features() const {
    return mFeatures;
}

bool BitInOutFormat::hasFeature( FormatFeatures feature ) const {
    return ( mFeatures & bitset< FEATURES_COUNT >( feature ) ) != 0;
}
