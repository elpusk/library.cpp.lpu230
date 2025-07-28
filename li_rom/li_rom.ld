# li_rom.ld
{
    global:
		tg_rom_load_header;
		tg_rom_get_updatable_item_index;
		tg_rom_get_item;
		tg_rom_readBinary_of_item;
		tg_rom_create_header;
		tg_rom_add_item;
		_so_init;
		_so_fini;
    local:
        *;
};
