#include <stdio.h>

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <imgui_memory_editor.h>

#include <mg/data/mzp.hpp>
#include <mg/data/mzx.hpp>
#include <mg/data/nam.hpp>
#include <mg/util/endian.hpp>
#include <mg/util/fs.hpp>
#include <mg/util/string.hpp>

enum DataType {
  UNDEFINED,
  DATA_NAM,
  DATA_MZP,
  DATA_MZX,
  DATA_HEXDUMP,
  DATA_STRING_TABLE,
};

struct DataFile;
std::vector<std::shared_ptr<DataFile>> data_file_contexts;

struct DataFile {
  std::string file_name;
  std::string raw_data;
  DataType display_type = UNDEFINED;

  DataFile *parent;
  unsigned parent_string_table_idx = -1;

  bool parsed_data_valid;
  void *parsed_data;

  MemoryEditor mem_edit;

  bool render() {
    // Data type selectable
    ImGui::Text("Interpret data as:");
    if (ImGui::Selectable("Hexdump", display_type == DATA_HEXDUMP)) {
      display_type = DATA_HEXDUMP;
      mem_edit.ReadOnly = true;
      parsed_data_valid = true;
    }
    if (ImGui::Selectable("NAM", display_type == DATA_NAM)) {
      display_type = DATA_NAM;
      parsed_data = new mg::data::Nam;
      parsed_data_valid = mg::data::nam_read(
          raw_data, *reinterpret_cast<mg::data::Nam *>(parsed_data));
    }
    if (ImGui::Selectable("MZP", display_type == DATA_MZP)) {
      display_type = DATA_MZP;
      parsed_data = new mg::data::Mzp;
      parsed_data_valid = mg::data::mzp_read(
          raw_data, *reinterpret_cast<mg::data::Mzp *>(parsed_data));
    }
    if (ImGui::Selectable("MZX", display_type == DATA_MZX)) {
      display_type = DATA_MZX;
      parsed_data = new std::string;
      parsed_data_valid = mg::data::mzx_decompress(
          raw_data, *reinterpret_cast<std::string *>(parsed_data));
    }
    if (ImGui::Selectable("String Table", display_type == DATA_STRING_TABLE)) {
      display_type = DATA_STRING_TABLE;
      parsed_data = new std::vector<std::string>;
      parsed_data_valid = true;
    }

    // Delegate detail render
    if (!parsed_data_valid) {
      ImGui::Text("Cannot interpret data this way.");
      return false;
    }

    switch (display_type) {
    case DATA_NAM:
      return render_nam();
    case DATA_MZP:
      return render_mzp();
    case DATA_MZX:
      return render_mzx();
    case DATA_HEXDUMP:
      return render_hex();
    case DATA_STRING_TABLE:
      return render_string_table();
    default:
      return false;
    }
  }

  bool render_string_table() {
    if (parent == nullptr) {
      ImGui::Text("No parent to retrieve string table from");
      return false;
    }

    if (parent->display_type != DATA_MZP) {
      ImGui::Text("Parent is not of type MZP");
      return false;
    }

    // Generate string table options
    mg::data::Mzp *parent_mzp =
        reinterpret_cast<mg::data::Mzp *>(parent->parsed_data);
    for (unsigned i = 0; i < parent_mzp->entry_headers.size(); i++) {
      if (ImGui::Selectable(mg::string::format("Entry %u", i).c_str(),
                            parent_string_table_idx == i)) {
        parent_string_table_idx = i;

        // Re-extract string table indices
        // Note string tables are big endian
        uint32_t *offsets =
            reinterpret_cast<uint32_t *>(parent_mzp->entry_data[i].data());
        const unsigned offset_count =
            parent_mzp->entry_data[i].size() / sizeof(uint32_t);
        auto *extracted_strings = new std::vector<std::string>;
        for (unsigned i = 0; i < offset_count; i++) {
          uint32_t start_offset = mg::be_to_host_u32(offsets[i]);
          uint32_t end_offset = i < offset_count - 1
                                    ? mg::be_to_host_u32(offsets[i + 1])
                                    : raw_data.size();
          if (start_offset >= raw_data.size() || end_offset > raw_data.size()) {
            extracted_strings->emplace_back(mg::string::format(
                "Invalid offset range %08x - %08x", start_offset, end_offset));
            continue;
          }
          extracted_strings->emplace_back(&raw_data[start_offset],
                                          end_offset - start_offset);
        }
        parsed_data = extracted_strings;
      }
    }

    int i = 0;
    for (auto &str :
         *reinterpret_cast<std::vector<std::string> *>(parsed_data)) {
      ImGui::Separator();
      ImGui::Text("%d", i++);
      ImGui::Text(str.c_str());
    }

    return false;
  }

  bool render_hex() {
    mem_edit.DrawContents(raw_data.data(), raw_data.size());
    return false;
  }

  bool render_nam() {
    mg::data::Nam *nam = reinterpret_cast<mg::data::Nam *>(parsed_data);
    for (auto &name : nam->names) {
      ImGui::Text(name.c_str());
    }
    return false;
  }

  bool render_mzp() {
    mg::data::Mzp *mzp = reinterpret_cast<mg::data::Mzp *>(parsed_data);
    ImGui::Text(
        mg::string::format("MZP with %u entries", mzp->entry_headers.size())
            .c_str());

    bool did_add_ctx = false;
    for (unsigned i = 0; i < mzp->entry_headers.size(); i++) {
      auto &entry = mzp->entry_headers[i];
      ImGui::PushID(i);
      ImGui::Text("Entry %4u of size %08x offset %08x", i,
                  entry.entry_data_size(),
                  mzp->archive_entry_start_offset(entry));
      ImGui::SameLine();
      if (ImGui::Button("Open Subarchive")) {
        // Create a new datafile context
        auto ctx = std::make_unique<DataFile>();
        ctx->file_name =
            mg::string::format("%s (MZP) @ %u", file_name.c_str(), i);
        ctx->raw_data = mzp->entry_data[i];
        ctx->parent = this;
        data_file_contexts.emplace_back(std::move(ctx));
        did_add_ctx = true;
      }
      ImGui::PopID();
    }

    return did_add_ctx;
  }

  bool render_mzx() {
    std::string *data = reinterpret_cast<std::string *>(parsed_data);
    mem_edit.ReadOnly = true;
    mem_edit.DrawContents(data->data(), data->size());

    return false;
  }
};

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s data_file\n", argv[0]);
    return -1;
  }

  // GL backend init
  glfwSetErrorCallback([](int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %sjn", error, description);
  });
  if (!glfwInit()) {
    return -1;
  }
  GLFWwindow *window =
      glfwCreateWindow(1920, 1080, "Data Explorrer", nullptr, nullptr);
  if (!window) {
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  // Render bindings
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.0;
  io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/unifont/unifont.ttf",
                               16, nullptr, io.Fonts->GetGlyphRangesJapanese());

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL2_Init();

  // Emplace the root data file
  auto root_file = std::make_unique<DataFile>();
  root_file->file_name = argv[1];
  if (!mg::fs::read_file(argv[1], root_file->raw_data)) {
    fprintf(stderr, "Failed to load root file\n");
    return -1;
  }
  data_file_contexts.emplace_back(std::move(root_file));

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Display a window for each file
    for (auto &context : data_file_contexts) {
      ImGui::Begin(context->file_name.c_str());
      const bool contexts_changed = context->render();
      ImGui::End();
      if (contexts_changed) {
        break;
      }
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
