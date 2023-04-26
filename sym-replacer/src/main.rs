use std::collections::HashMap;

use goblin::mach::{load_command::CommandVariant, symbols::Nlist64};
use serde::Deserialize;

#[macro_use]
extern crate log;

fn copy_dir_all(src: impl AsRef<std::path::Path>, dst: impl AsRef<std::path::Path>) {
    std::fs::create_dir_all(&dst).unwrap();
    for entry in std::fs::read_dir(src).unwrap().flatten() {
        if entry.file_type().unwrap().is_dir() {
            copy_dir_all(entry.path(), dst.as_ref().join(entry.file_name()));
        } else {
            std::fs::copy(entry.path(), dst.as_ref().join(entry.file_name())).unwrap();
        }
    }
}

#[derive(Deserialize)]
struct Data {
    pub name: String,
    pub orig2fixed: HashMap<String, String>,
}

fn main() {
    simple_logger::init().unwrap();

    let Some(kext_path) = std::env::args().nth(1).map(std::path::PathBuf::from) else {
        error!("Usage: {} <kext_path> <data>", std::env::args().next().unwrap());
        return;
    };
    let Some(data_path) = std::env::args().nth(2).map(std::path::PathBuf::from) else {
        error!("Usage: {} <kext_path> <data>", std::env::args().next().unwrap());
        return;
    };
    let data: Data = ron::from_str(&std::fs::read_to_string(data_path).unwrap()).unwrap();
    info!("{}", data.name);
    let kext_name = kext_path.file_stem().unwrap();
    let exec_data =
        std::fs::read(kext_path.join("Contents").join("MacOS").join(kext_name)).unwrap();
    let obj = goblin::Object::parse(&exec_data).unwrap();
    let goblin::Object::Mach(goblin::mach::Mach::Binary(macho)) = obj else {
        panic!("not a Mach-O binary");
    };
    assert!(macho.is_64);
    let (cmd_off, nsyms, symoff) = macho
        .load_commands
        .iter()
        .find_map(|cmd| {
            let CommandVariant::Symtab(v) = cmd.command else {
                return None;
            };
            Some((cmd.offset, v.nsyms as usize, v.symoff as usize))
        })
        .unwrap();

    let mut new_str_table = vec![0u8; 4];
    let mut off2fixed = HashMap::new();
    for (sym, off) in macho
        .symbols()
        .flatten()
        .map(|(sym, nlist)| (sym, nlist.n_strx))
    {
        off2fixed.insert(off, new_str_table.len());
        let v = data.orig2fixed.get(sym).map(|v| v.as_str()).unwrap_or(sym);
        if sym != v {
            info!("{sym} -> {v}");
        }
        new_str_table.extend_from_slice(v.as_bytes());
        new_str_table.push(0);
    }
    new_str_table.extend_from_slice(&[0u8; 4]);

    let mut new_data = exec_data.clone();
    new_data[cmd_off + 16..cmd_off + 20].copy_from_slice(&(exec_data.len() as u32).to_le_bytes()); // stroff
    new_data[cmd_off + 20..cmd_off + 24]
        .copy_from_slice(&(new_str_table.len() as u32).to_le_bytes()); // strsize
    new_data.extend_from_slice(&new_str_table);
    for i in 0..nsyms {
        let nlist_off = symoff + i * std::mem::size_of::<Nlist64>();
        let u_un = u32::from_le_bytes(new_data[nlist_off..nlist_off + 4].try_into().unwrap());
        new_data[nlist_off..nlist_off + 4]
            .copy_from_slice(&(*off2fixed.get(&(u_un as usize)).unwrap() as u32).to_le_bytes())
    }
    let result_path = kext_path.join("..").join("Results");
    copy_dir_all(
        kext_path.clone(),
        result_path.join(kext_path.file_name().unwrap()),
    );
    std::fs::write(
        result_path
            .join(kext_path.file_name().unwrap())
            .join("Contents")
            .join("MacOS")
            .join(kext_name),
        new_data,
    )
    .unwrap();
    info!("Done");
}
