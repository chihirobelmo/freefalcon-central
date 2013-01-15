/*******************************************************************************\
	Read in the tile tool's binary database (generated by TileTool) and the
	list of texture codes used for this map (generated by TexMunge) and write out
	the runtime texture database for Falcon 4.0

	Scott Randolph
	Spectrum HoloByte
	February 15, 1996
\*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "TileDB.h"


int main(int argc, char **argv) {

	int				i, j;
	TileDatabase	tileDB;
	int				zero = 0;
	DWORD			bytes;
	char			source[256];
	char			filename[256];
	char			basename[20];
	int				id;
	FILE			*listFile;
	HANDLE			outputFile;
	HANDLE			checkFile;
	int				set;
	int				numSets;
	int				maxSet = 0;
	int				tile;
	int				totalTiles = 0;
	int				nareas;
	int				npaths;
	TileRecord		*pTile;
	AreaRecord		*pArea;
	PathRecord		*pPath;

	const int		codeListLen = 4096;
	struct {
		WORD	newCode;
		WORD	originalCode;
		char	name[20];
	}				codeList[codeListLen];

	const int		setListLen = 256;
#pragma pack (push, 1)
	struct {
		int		numTiles;
		BYTE	terrainType;
	}				setList[setListLen];
#pragma pack (pop)

	// Make sure we got the right number of parameters
	if ((argc != 2) || (*(argv[1]+strlen(argv[1])-1) != '\\')) {
		printf("Usage:  TexGen <texture directory>\n");
		printf("        (the directory should be terminated with a \\ character.)\n");
		printf("        The files TILES.BDB and TEXCODES.TXT are read as input.\n");
		printf("        The file TEXTURE.BIN is written as output.\n");
		return -1;
	}


	// Load the tile database from the texture directory
	strcpy( source, argv[1] );
	strcat( source, "tiles.bdb" );
	printf( "Loading tile database %s\n", source );
	tileDB.Load( source );


	// Open the list of textures to the user provided
	strcpy( source, argv[1] );
	strcat( source, "TexCodes.txt" );
	printf( "Reading from list file %s\n", source );

	listFile = fopen( source, "r" );
	ShiAssert( listFile );


	// Loop until the input file is exhausted
	int		ret;
	char	codeString[8];
	while ((ret = fscanf( listFile, "%X %s %*[^\n]", &id, &basename )) == 2) {
		codeList[totalTiles].newCode = id;
		strcpy( codeList[totalTiles].name, basename );

		// Extract the original tile code from the file name
		strcpy( codeString, "0x" );
		strncpy( &codeString[2], &basename[5], 3 );
		sscanf( codeString, "%hx", &codeList[totalTiles].originalCode );

		totalTiles++;
		ShiAssert( totalTiles < codeListLen );
	}
	
	// Back off one tile (the last one is the end marker)
	totalTiles--;


	// Now scan the list to count sets and tiles within sets
	int prevSet = -1;
	for (i=0; i<totalTiles; i++) {

		// Ensure this tile's bitmap file exists (check all three levels)
		strcpy( basename, codeList[i].name );
		strcpy( filename, argv[1] );
		strcat( filename, basename );
		checkFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (checkFile == INVALID_HANDLE_VALUE) {
			char message[256];
			char errorText[256];
			PutErrorString( errorText );
			sprintf( message, "Couldn't open %s:  %s", basename, errorText );
			MessageBox( NULL, message, "Missing File", MB_OK );
		}
		CloseHandle( checkFile );
		
		basename[0] = 'M';
		strcpy( filename, argv[1] );
		strcat( filename, basename );
		checkFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (checkFile == INVALID_HANDLE_VALUE) {
			char message[256];
			char errorText[256];
			PutErrorString( errorText );
			sprintf( message, "Couldn't open %s:  %s", basename, errorText );
			MessageBox( NULL, message, "Missing File", MB_OK );
		}
		CloseHandle( checkFile );

		basename[0] = 'L';
		strcpy( filename, argv[1] );
		strcat( filename, basename );
		checkFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (checkFile == INVALID_HANDLE_VALUE) {
			char message[256];
			char errorText[256];
			PutErrorString( errorText );
			sprintf( message, "Couldn't open %s:  %s", basename, errorText );
			MessageBox( NULL, message, "Missing File", MB_OK );
		}
		CloseHandle( checkFile );


		// Extract the set number from the tile code
		set = codeList[i].newCode >> 4;

		if (prevSet != set) {
			// We encountered a new tile set
			ShiAssert( set < setListLen );

			// Keep track of the sets we've seen
			maxSet = set;

			// Initialize the tile counts to zero
			while (prevSet < set) {
				prevSet++;
				setList[prevSet].numTiles = 0;
				setList[prevSet].terrainType = 0;
			}

			// Decide what tile type we are
			pTile = tileDB.GetTileRecord( codeList[i].originalCode );
			setList[set].numTiles = 1;
			setList[set].terrainType  = tileDB.GetTerrainType( pTile );
		} else {
			// We are continueing in an old set
			ShiAssert( set < setListLen );
			setList[set].numTiles++;
		}
	}


	// Open the texture database file for writing
	strcpy( source, argv[1] );
	strcat( source, "Texture.bin" );
	outputFile = CreateFile( source, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (outputFile == INVALID_HANDLE_VALUE) {
		char	string[80];
		PutErrorString( string );
		ShiError( string );
	}


	// Store the number of sets and the total number of tiles
	numSets = maxSet + 1;
	WriteFile( outputFile, &numSets, sizeof(numSets), &bytes, NULL );
	WriteFile( outputFile, &totalTiles, sizeof(totalTiles), &bytes, NULL );


	// Make the write out scan for the tile set list
	tile = 0;
	for (set=0; set<=maxSet; set++) {

		// Write the number of tiles in this set and the terrain type
		WriteFile( outputFile, &setList[set], sizeof(setList[set]), &bytes, NULL );

		// Write out the list of tile names and their areas and paths
		for (i=0; i<setList[set].numTiles; i++, tile++) {

			ShiAssert( tile < totalTiles );

			// Write this tile name
			WriteFile( outputFile, &codeList[tile].name, sizeof(codeList[tile].name), &bytes, NULL );

			// Get the tile record
			pTile = tileDB.GetTileRecord( codeList[tile].originalCode );

			// Write the number of areas and paths on this tile
			nareas = tileDB.GetNAreas( pTile );
			npaths = tileDB.GetNPaths( pTile );
			WriteFile( outputFile, &nareas, sizeof(nareas), &bytes, NULL );
			WriteFile( outputFile, &npaths, sizeof(npaths), &bytes, NULL );

			// Write out the area records
			for (j=0; j< nareas; j++) {
				pArea = tileDB.GetArea( pTile, j );
				ShiAssert( pArea );
				WriteFile( outputFile, pArea, sizeof(*pArea), &bytes, NULL );
			}

			// Write out the path records
			for (j=0; j< npaths; j++) {
				pPath = tileDB.GetPath( pTile, j );
				ShiAssert( pPath );
				WriteFile( outputFile, pPath, sizeof(*pPath), &bytes, NULL );
			}
		}
	}


	// Close the tile set list file
	CloseHandle( outputFile );

	// Release the tile database memory
	tileDB.Free();

	return 0;
}