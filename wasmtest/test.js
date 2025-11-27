
import { TinyLayoutEngineWASM } from "./build/tinyLayoutEngine.mjs";

let wasm = null; 
loadWasm = async () => {
    try {

        if(!wasm){
            let config = {
                locateFile:(path, prefix)=>{
                    return path; //just return the path part so the file is loaded from root
                }
            }; 
            wasm = await TinyLayoutEngineWASM(config);
        }

    } catch(err) { 
        console.error(`Unexpected error in TinyLayoutEngine.loadWasm. [Message: ${err.message}]`);
        throw {message: `Unexpected error in TinyLayoutEngine.loadWasm. [Message: ${err.message}]`}; 
    }
}

window.onload = async ()=>{
    
    await loadWasm();

    const canvas1 = document.getElementById("test-canvas-1");
    const canvas2 = document.getElementById("test-canvas-2");
    const canvas3 = document.getElementById("test-canvas-3");
    const canvas4 = document.getElementById("test-canvas-4");

    

}