import tkinter as tk
from tkinter import ttk, colorchooser
from colorsys import rgb_to_hls, hls_to_rgb
from PIL import Image, ImageTk # For GIF animation
import random
import time # For simulating timestamps in log

# --- Color Definitions (based on user request and complementary colors) ---
# Base colors (vibrant, cyberpunk/neon feel)
COLOR_ORANGE = "#FFA500" # Vibrant Orange
COLOR_CYAN = "#00FFFF"   # Bright Cyan
COLOR_CORAL = "#FF7F50"  # Warm Coral
COLOR_VIOLET = "#EE82EE" # Soft Violet

# Complementary colors (adjusted for aesthetic blend and pop)
COMPLEMENT_ORANGE_BLUE = "#005AFF" # Strong Blue
COMPLEMENT_CYAN_RED = "#FF0000"   # Pure Red
COMPLEMENT_CORAL_TEAL = "#0080AF" # Deep Teal
COMPLEMENT_VIOLET_LIME = "#7FFF00" # Vibrant Lime Green

# Derived UI colors for a glassmorphism feel (fogged grey background with light blue tint)
# Main background of the window (increased opacity, faded blue-grey for "fogged" look)
UI_MAIN_BG = "#353A42" # Dark blue-grey for the main background
UI_MAIN_ALPHA = 0.95 # Increased opacity for a more "fogged" effect

# Background for "glass" panels (slightly lighter, more opaque to stand out)
UI_PANEL_BG = "#4A505A" # Lighter blue-grey for panels, maintains distinction
UI_PANEL_ALPHA = 0.65 # Reduced opacity for panel transparency effect over main BG

# Text and Accent Colors (maintaining contrast and neon feel)
UI_TEXT_COLOR = "#E0E0E0" # Light grey for general text
UI_LIGHT_TEXT_COLOR = "#AAAAAA" # Lighter grey for secondary text
UI_ACCENT_COLOR_1 = COLOR_ORANGE # Primary accent
UI_ACCENT_COLOR_2 = COLOR_CYAN   # Secondary accent
UI_ACCENT_COLOR_3 = COLOR_CORAL  # Tertiary accent
UI_ACCENT_COLOR_4 = COLOR_VIOLET # Quaternary accent
UI_ERROR_COLOR = COMPLEMENT_CYAN_RED # For error messages in log


# Function to get complementary color (basic HSL shift) - kept for reference if needed
def get_complementary_hex(hex_color):
    hex_color = hex_color.lstrip('#')
    rgb = tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))
    r_norm, g_norm, b_norm = [x / 255.0 for x in rgb]
    h, l, s = rgb_to_hls(r_norm, g_norm, b_norm)
    complementary_h = (h + 0.5) % 1.0
    comp_r, comp_g, comp_b = hls_to_rgb(complementary_h, l, s)
    comp_rgb = tuple(int(x * 255) for x in (comp_r, comp_g, comp_b))
    return '#%02X%02X%02X' % comp_rgb

class EmoBotControllerApp:
    def __init__(self, master):
        self.master = master
        master.title("Emo Bot Controller")
        master.geometry("1200x850") # Increased size for more content
        master.resizable(False, False) # Fixed size for aesthetic consistency

        # Set overall window transparency (requires OS support, e.g., Windows, some Linux WMs)
        try:
            master.attributes("-alpha", UI_MAIN_ALPHA) # Semi-transparent window
            master.attributes("-transparentcolor", UI_MAIN_BG) # Make main window background transparent
            master.config(bg=UI_MAIN_BG) # Set the background color
        except tk.TclError:
            print("Warning: Window transparency features may not be supported on this OS/WM. Falling back to opaque.")
            master.attributes("-alpha", 1.0) # Fallback to fully opaque
            master.config(bg=UI_MAIN_BG)

        # Style Configuration for Glassmorphism effect
        self.style = ttk.Style()
        self.style.theme_use('clam') # 'clam' is often good for customization

        # Custom style for "glass" frames - panels that appear on top of the main background
        self.style.configure("Glass.TFrame",
                             background=UI_PANEL_BG,
                             relief="flat",
                             borderwidth=0,
                             padding=15) # Increased padding for softer corners visually

        # Custom style for labels
        self.style.configure("Glass.TLabel",
                             background=UI_PANEL_BG,
                             foreground=UI_TEXT_COLOR,
                             font=("Inter", 12))
        self.style.configure("Glass.Heading.TLabel", # For section titles
                             background=UI_PANEL_BG,
                             foreground=UI_ACCENT_COLOR_1, # Use an accent color for headings
                             font=("Inter", 16, "bold"))
        self.style.configure("Glass.SubHeading.TLabel", # For sub-sections
                             background=UI_PANEL_BG,
                             foreground=UI_TEXT_COLOR,
                             font=("Inter", 13, "bold"))
        self.style.configure("Glass.Small.TLabel", # For smaller info text
                             background=UI_PANEL_BG,
                             foreground=UI_LIGHT_TEXT_COLOR,
                             font=("Inter", 10))

        # Custom style for entry widgets (simulated glass)
        self.style.configure("Glass.TEntry",
                             fieldbackground="#4A4A4A", # Darker than panel for input, contrasts with foreground
                             foreground=UI_ACCENT_COLOR_2, # Accent color for input text
                             insertcolor=UI_ACCENT_COLOR_2,
                             borderwidth=1,
                             relief="solid",
                             padding=5,
                             font=("Inter", 11))

        # Custom style for buttons
        self.style.configure("Glass.TButton",
                             background=UI_ACCENT_COLOR_3, # Coral for general buttons
                             foreground=UI_MAIN_BG, # Dark text on bright button
                             font=("Inter", 11, "bold"),
                             relief="flat",
                             borderwidth=0,
                             padding=10,
                             focusthickness=0)
        self.style.map("Glass.TButton",
                       background=[('active', COMPLEMENT_ORANGE_BLUE), ('pressed', UI_ACCENT_COLOR_4)], # More vibrant interaction
                       foreground=[('active', UI_TEXT_COLOR)]) # Text becomes light on active

        # Custom style for Sliders (Scales)
        self.style.configure("Glass.Horizontal.TScale",
                             background=UI_PANEL_BG,
                             foreground=UI_ACCENT_COLOR_1, # Orange for slider values
                             troughcolor="#4A4A4A", # Darker track
                             sliderrelief="flat",
                             sliderthickness=20,
                             troughrelief="flat",
                             borderwidth=0)
        self.style.map("Glass.Horizontal.TScale",
                       background=[('active', COMPLEMENT_ORANGE_BLUE)]) # Blue for active slider

        # Custom style for Checkbuttons / Radiobuttons (toggle-like)
        self.style.configure("Glass.TCheckbutton",
                             background=UI_PANEL_BG,
                             foreground=UI_TEXT_COLOR,
                             font=("Inter", 12),
                             indicatoron=False, # Make it look more like a toggle button
                             relief="flat",
                             padding=5)
        self.style.map("Glass.TCheckbutton",
                       background=[('selected', UI_ACCENT_COLOR_4), ('active', UI_ACCENT_COLOR_3)],
                       foreground=[('selected', UI_MAIN_BG), ('active', UI_TEXT_COLOR)]) # Text becomes light on active/selected

        # Custom style for Combobox
        self.style.configure("Glass.TCombobox",
                             fieldbackground="#4A4A4A",
                             background=UI_ACCENT_COLOR_2, # Cyan for dropdown button
                             foreground=UI_MAIN_BG,
                             selectbackground=COMPLEMENT_ORANGE_BLUE, # Blue for selection highlight
                             selectforeground=UI_TEXT_COLOR,
                             padding=5,
                             font=("Inter", 11),
                             relief="flat")
        self.style.map("Glass.TCombobox",
                       background=[('readonly', UI_ACCENT_COLOR_2)],
                       fieldbackground=[('readonly', "#4A4A4A")],
                       foreground=[('readonly', UI_TEXT_COLOR)]) # Text color in combobox

        # Custom style for Text widget (for chat/log)
        self.style.configure("Glass.TText", # Note: Tkinter Text widget doesn't use ttk styles directly for background/foreground
                             background="#2A2A2A", # Darker than panels for log/chat
                             foreground=UI_TEXT_COLOR,
                             insertbackground=UI_ACCENT_COLOR_2,
                             selectbackground=COMPLEMENT_ORANGE_BLUE,
                             selectforeground=UI_TEXT_COLOR)

        # Main Layout Frame with Grid for Multi-layered UI
        self.main_frame = tk.Frame(master, bg=UI_MAIN_BG)
        self.main_frame.pack(fill="both", expand=True, padx=20, pady=20)
        self.main_frame.grid_columnconfigure(0, weight=1)
        self.main_frame.grid_columnconfigure(1, weight=2) # Center column wider for visualizer
        self.main_frame.grid_columnconfigure(2, weight=1)
        self.main_frame.grid_rowconfigure(0, weight=1)
        self.main_frame.grid_rowconfigure(1, weight=1)

        # --- Left Column: Connection & Network ---
        left_column_frame = ttk.Frame(self.main_frame, style="Glass.TFrame")
        left_column_frame.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)
        left_column_frame.grid_columnconfigure(0, weight=1)

        # Bot Status & General Controls
        self.status_frame = ttk.Frame(left_column_frame, style="Glass.TFrame")
        self.status_frame.pack(pady=10, fill="x")
        ttk.Label(self.status_frame, text="System Status", style="Glass.Heading.TLabel").pack(pady=(0, 10))
        self.status_label = ttk.Label(self.status_frame, text="Connection: Disconnected", style="Glass.TLabel", foreground=COMPLEMENT_CYAN_RED) # Initial red for disconnected
        self.status_label.pack(side="left", padx=5, pady=5)
        ttk.Button(self.status_frame, text="Connect", command=self.connect_bot, style="Glass.TButton").pack(side="right", padx=5, pady=5)
        ttk.Button(self.status_frame, text="Disconnect", command=self.disconnect_bot, style="Glass.TButton").pack(side="right", padx=5, pady=5)

        # WiFi Setup
        self.wifi_frame = ttk.Frame(left_column_frame, style="Glass.TFrame")
        self.wifi_frame.pack(pady=10, fill="x")
        ttk.Label(self.wifi_frame, text="Network Configuration", style="Glass.SubHeading.TLabel").pack(pady=(0, 10))

        ttk.Label(self.wifi_frame, text="WiFi SSID:", style="Glass.TLabel").pack(anchor="w", padx=5, pady=2)
        self.wifi_ssid_entry = ttk.Entry(self.wifi_frame, style="Glass.TEntry")
        self.wifi_ssid_entry.insert(0, "NeoTokyo_Net")
        self.wifi_ssid_entry.pack(fill="x", padx=5, pady=2)

        ttk.Label(self.wifi_frame, text="Password:", style="Glass.TLabel").pack(anchor="w", padx=5, pady=2)
        self.wifi_password_entry = ttk.Entry(self.wifi_frame, show="*", style="Glass.TEntry")
        self.wifi_password_entry.insert(0, "************")
        self.wifi_password_entry.pack(fill="x", padx=5, pady=2)
        ttk.Button(self.wifi_frame, text="Apply WiFi Settings", command=self.apply_wifi_settings, style="Glass.TButton").pack(pady=10)

        # Network Info
        self.network_info_frame = ttk.Frame(left_column_frame, style="Glass.TFrame")
        self.network_info_frame.pack(pady=10, fill="x")
        ttk.Label(self.network_info_frame, text="Network Information", style="Glass.SubHeading.TLabel").pack(pady=(0, 10))
        self.ip_label = ttk.Label(self.network_info_frame, text="IP Address: 192.168.1.100", style="Glass.TLabel")
        self.ip_label.pack(anchor="w", padx=5, pady=2)
        self.mac_label = ttk.Label(self.network_info_frame, text="MAC Address: 00:1A:2B:3C:4D:5E", style="Glass.TLabel")
        self.mac_label.pack(anchor="w", padx=5, pady=2)
        self.hostname_label = ttk.Label(self.network_info_frame, text="Hostname: EMO-BOT-001", style="Glass.TLabel")
        self.hostname_label.pack(anchor="w", padx=5, pady=2)
        # New: Bluetooth Pairing Button
        ttk.Button(self.network_info_frame, text="Pair Bluetooth Device", command=self.pair_bluetooth, style="Glass.TButton").pack(pady=5)
        # New: Battery Level Display
        self.battery_label = ttk.Label(self.network_info_frame, text="Battery: 78%", style="Glass.TLabel", foreground=COMPLEMENT_VIOLET_LIME)
        self.battery_label.pack(anchor="w", padx=5, pady=2)
        ttk.Button(self.network_info_frame, text="Check Battery", command=self.check_battery, style="Glass.TButton").pack(pady=5)


        # --- Center Column (spanning two rows): 3D Model & Appearance ---
        center_column_frame = ttk.Frame(self.main_frame, style="Glass.TFrame")
        center_column_frame.grid(row=0, column=1, rowspan=2, sticky="nsew", padx=10, pady=10)
        center_column_frame.grid_columnconfigure(0, weight=1)

        # 3D Model Visualizer (now with GIF)
        ttk.Label(center_column_frame, text="Bot Visualizer", style="Glass.Heading.TLabel").pack(pady=(0, 10))
        self.model_canvas = tk.Canvas(center_column_frame, bg="#111111", highlightthickness=0, relief="solid")
        self.model_canvas.pack(fill="both", expand=True, padx=10, pady=10)
        # For rounded corners, we draw a rounded rectangle on the canvas itself
        self.canvas_bg_rect = self.model_canvas.create_rectangle(
            0, 0, self.model_canvas.winfo_width(), self.model_canvas.winfo_height(),
            fill="#111111", outline="", tags="background_rect"
        )
        self.model_canvas.bind("<Configure>", self._resize_canvas_bg) # Adjust rectangle on resize
        # GIF Animation setup
        self.gif_path = "C:\\Users\\91936\\Downloads\\animatron.gif" # User must place this file next to the script
        self.gif_frames = []
        self.gif_frame_idx = 0
        self.gif_delay = 100 # Milliseconds per frame (adjust as needed)
        try:
            self.load_gif_frames()
            self.gif_image_on_canvas = self.model_canvas.create_image(
                self.model_canvas.winfo_width() / 2, self.model_canvas.winfo_height() / 2,
                image=self.gif_frames[0], tags="gif_image"
            )
            self.animate_gif()
        except FileNotFoundError:
            self.model_canvas.create_text(
                self.model_canvas.winfo_width() / 2, self.model_canvas.winfo_height() / 2 - 20,
                text="animatron.gif not found!", fill=UI_ERROR_COLOR, font=("Inter", 12, "bold")
            )
            self.model_canvas.create_text(
                self.model_canvas.winfo_width() / 2, self.model_canvas.winfo_height() / 2 + 10,
                text="Please place 'animatron.gif' in the same directory.", fill=UI_LIGHT_TEXT_COLOR, font=("Inter", 10)
            )
        self.model_canvas.create_text(
            self.model_canvas.winfo_width() / 2, self.model_canvas.winfo_height() / 2 + 100,
            text="(Future Interactive 3D Model)", fill=UI_LIGHT_TEXT_COLOR, font=("Inter", 10)
        )

        # Bot Appearance Section
        self.appearance_frame = ttk.Frame(center_column_frame, style="Glass.TFrame")
        self.appearance_frame.pack(pady=10, fill="x")
        ttk.Label(self.appearance_frame, text="Appearance Customization", style="Glass.SubHeading.TLabel").pack(pady=(0, 10))

        name_frame = ttk.Frame(self.appearance_frame, style="Glass.TFrame")
        name_frame.pack(fill="x", pady=5)
        ttk.Label(name_frame, text="Bot Name:", style="Glass.TLabel").pack(side="left", padx=5)
        self.bot_name_entry = ttk.Entry(name_frame, style="Glass.TEntry")
        self.bot_name_entry.insert(0, "A.K.I.R.A Unit 7") # More Akira-like name
        self.bot_name_entry.pack(side="left", expand=True, fill="x", padx=5)
        ttk.Button(name_frame, text="Set Name", command=self.set_bot_name, style="Glass.TButton").pack(side="right", padx=5)

        color_frame = ttk.Frame(self.appearance_frame, style="Glass.TFrame")
        color_frame.pack(fill="x", pady=5)
        ttk.Label(color_frame, text="Eye/LED Color:", style="Glass.TLabel").pack(side="left", padx=5)
        self.current_eye_color = tk.StringVar(value=COLOR_CYAN) # Default Cyan
        self.eye_color_display = tk.Label(color_frame, bg=self.current_eye_color.get(), width=3, relief="solid", bd=1)
        self.eye_color_display.pack(side="left", padx=5)
        ttk.Button(color_frame, text="Pick Color", command=self.pick_eye_color, style="Glass.TButton").pack(side="left", padx=5)
        ttk.Button(color_frame, text="Apply Color", command=self.apply_eye_color, style="Glass.TButton").pack(side="left", padx=5)

        accessory_frame = ttk.Frame(self.appearance_frame, style="Glass.TFrame")
        accessory_frame.pack(fill="x", pady=5)
        ttk.Label(accessory_frame, text="Accessory:", style="Glass.TLabel").pack(side="left", padx=5)
        self.accessory_options = ["None", "Mini Hat", "Bowtie", "Headphones", "Glasses", "Cyber-Visor"] # Added one
        self.selected_accessory = tk.StringVar(value=self.accessory_options[0])
        self.accessory_combobox = ttk.Combobox(accessory_frame,
                                               textvariable=self.selected_accessory,
                                               values=self.accessory_options,
                                               state="readonly",
                                               style="Glass.TCombobox")
        self.accessory_combobox.pack(side="left", expand=True, fill="x", padx=5)
        self.accessory_combobox.bind("<<ComboboxSelected>>", self.apply_accessory)

        # New: Robot Body Material/Texture
        material_frame = ttk.Frame(self.appearance_frame, style="Glass.TFrame")
        material_frame.pack(fill="x", pady=5)
        ttk.Label(material_frame, text="Body Material:", style="Glass.TLabel").pack(side="left", padx=5)
        self.material_options = ["Standard Alloy", "Glossy Black", "Matte Grey", "Chrome", "Carbon Fiber"]
        self.selected_material = tk.StringVar(value=self.material_options[0])
        self.material_combobox = ttk.Combobox(material_frame,
                                              textvariable=self.selected_material,
                                              values=self.material_options,
                                              state="readonly",
                                              style="Glass.TCombobox")
        self.material_combobox.pack(side="left", expand=True, fill="x", padx=5)
        self.material_combobox.bind("<<ComboboxSelected>>", self.apply_material)

        # New: Light Pattern Editor Button
        ttk.Button(self.appearance_frame, text="Open Light Pattern Editor", command=self.open_light_editor, style="Glass.TButton").pack(pady=5)


        # --- Right Column: Behavior, Audio, Log ---
        right_column_frame = ttk.Frame(self.main_frame, style="Glass.TFrame")
        right_column_frame.grid(row=0, column=2, sticky="nsew", padx=10, pady=10)
        right_column_frame.grid_columnconfigure(0, weight=1)


        # Bot Behavior Section
        self.behavior_frame = ttk.Frame(right_column_frame, style="Glass.TFrame")
        self.behavior_frame.pack(pady=10, fill="x")
        ttk.Label(self.behavior_frame, text="Behavior Controls", style="Glass.Heading.TLabel").pack(pady=(0, 10))

        mood_frame = ttk.Frame(self.behavior_frame, style="Glass.TFrame")
        mood_frame.pack(fill="x", pady=5)
        ttk.Label(mood_frame, text="Mood:", style="Glass.TLabel").pack(side="left", padx=5)
        self.mood_slider = ttk.Scale(mood_frame,
                                     from_=0, to=100,
                                     orient="horizontal",
                                     command=self.update_mood_label,
                                     style="Glass.Horizontal.TScale")
        self.mood_slider.set(50)
        self.mood_slider.pack(side="left", expand=True, fill="x", padx=5)
        self.mood_label = ttk.Label(mood_frame, text="Neutral (50%)", style="Glass.TLabel")
        self.mood_label.pack(side="left", padx=5)
        ttk.Button(mood_frame, text="Apply", command=self.apply_mood, style="Glass.TButton").pack(side="right", padx=5)

        interaction_frame = ttk.Frame(self.behavior_frame, style="Glass.TFrame")
        interaction_frame.pack(fill="x", pady=5)
        ttk.Label(interaction_frame, text="Interaction:", style="Glass.TLabel").pack(side="left", padx=5)
        self.interaction_options = ["Playful", "Reserved", "Responsive", "Curious", "Analytical"] # Added one
        self.selected_interaction = tk.StringVar(value=self.interaction_options[2])
        for text in self.interaction_options:
            rb = ttk.Radiobutton(interaction_frame, text=text, variable=self.selected_interaction,
                                 value=text, command=self.apply_interaction_style, style="Glass.TCheckbutton")
            rb.pack(side="left", padx=5)

        expressiveness_frame = ttk.Frame(self.behavior_frame, style="Glass.TFrame")
        expressiveness_frame.pack(fill="x", pady=5)
        ttk.Label(expressiveness_frame, text="Expressiveness:", style="Glass.TLabel").pack(side="left", padx=5)
        self.expressiveness_slider = ttk.Scale(expressiveness_frame, from_=0, to=100, orient="horizontal", style="Glass.Horizontal.TScale")
        self.expressiveness_slider.set(70)
        self.expressiveness_slider.pack(side="left", expand=True, fill="x", padx=5)
        self.expressiveness_label = ttk.Label(expressiveness_frame, text="70%", style="Glass.TLabel")
        self.expressiveness_label.pack(side="left", padx=5)
        self.expressiveness_slider.config(command=lambda v: self.expressiveness_label.config(text=f"{int(float(v))}%"))
        ttk.Button(expressiveness_frame, text="Apply", command=lambda: self.show_message("Expressiveness Set", f"Bot expressiveness set to {int(self.expressiveness_slider.get())}%."), style="Glass.TButton").pack(side="right", padx=5)

        self.sleep_mode_var = tk.BooleanVar(value=False)
        self.sleep_mode_check = ttk.Checkbutton(self.behavior_frame, text="Enable Sleep Mode",
                                                  variable=self.sleep_mode_var,
                                                  command=self.toggle_sleep_mode,
                                                  style="Glass.TCheckbutton")
        self.sleep_mode_check.pack(pady=5)

        self.follow_gaze_var = tk.BooleanVar(value=True)
        self.follow_gaze_check = ttk.Checkbutton(self.behavior_frame, text="Follow User Gaze",
                                                  variable=self.follow_gaze_var,
                                                  command=self.toggle_follow_gaze,
                                                  style="Glass.TCheckbutton")
        self.follow_gaze_check.pack(pady=5)

        self.auto_respond_var = tk.BooleanVar(value=True)
        self.auto_respond_check = ttk.Checkbutton(self.behavior_frame, text="Auto-Respond to Greetings",
                                                  variable=self.auto_respond_var,
                                                  command=self.toggle_auto_respond,
                                                  style="Glass.TCheckbutton")
        self.auto_respond_check.pack(pady=5)

        # New: Personal Space Sensitivity
        space_sens_frame = ttk.Frame(self.behavior_frame, style="Glass.TFrame")
        space_sens_frame.pack(fill="x", pady=5)
        ttk.Label(space_sens_frame, text="Space Sensitivity:", style="Glass.TLabel").pack(side="left", padx=5)
        self.space_sens_slider = ttk.Scale(space_sens_frame, from_=0, to=100, orient="horizontal", style="Glass.Horizontal.TScale")
        self.space_sens_slider.set(50)
        self.space_sens_slider.pack(side="left", expand=True, fill="x", padx=5)
        self.space_sens_label = ttk.Label(space_sens_frame, text="50%", style="Glass.TLabel")
        self.space_sens_label.pack(side="left", padx=5)
        self.space_sens_slider.config(command=lambda v: self.space_sens_label.config(text=f"{int(float(v))}%"))
        ttk.Button(space_sens_frame, text="Apply", command=lambda: self.show_message("Space Sensitivity Set", f"Personal space sensitivity set to {int(self.space_sens_slider.get())}%."), style="Glass.TButton").pack(side="right", padx=5)

        # New: Learning Mode Toggle
        self.learning_mode_var = tk.BooleanVar(value=True)
        self.learning_mode_check = ttk.Checkbutton(self.behavior_frame, text="Enable Learning Mode",
                                                    variable=self.learning_mode_var,
                                                    command=self.toggle_learning_mode,
                                                    style="Glass.TCheckbutton")
        self.learning_mode_check.pack(pady=5)

        # --- Audio & Animation Section ---
        self.audio_anim_frame = ttk.Frame(right_column_frame, style="Glass.TFrame")
        self.audio_anim_frame.pack(pady=10, fill="x")
        ttk.Label(self.audio_anim_frame, text="Audio & Animation", style="Glass.SubHeading.TLabel").pack(pady=(0, 10))

        volume_frame = ttk.Frame(self.audio_anim_frame, style="Glass.TFrame")
        volume_frame.pack(fill="x", pady=5)
        ttk.Label(volume_frame, text="Volume:", style="Glass.TLabel").pack(side="left", padx=5)
        self.volume_slider = ttk.Scale(volume_frame,
                                       from_=0, to=100,
                                       orient="horizontal",
                                       command=self.update_volume_label,
                                       style="Glass.Horizontal.TScale")
        self.volume_slider.set(75)
        self.volume_slider.pack(side="left", expand=True, fill="x", padx=5)
        self.volume_label = ttk.Label(volume_frame, text="75%", style="Glass.TLabel")
        self.volume_label.pack(side="left", padx=5)
        ttk.Button(volume_frame, text="Apply", command=self.apply_volume, style="Glass.TButton").pack(side="right", padx=5)

        anim_speed_frame = ttk.Frame(self.audio_anim_frame, style="Glass.TFrame")
        anim_speed_frame.pack(fill="x", pady=5)
        ttk.Label(anim_speed_frame, text="Animation Speed:", style="Glass.TLabel").pack(side="left", padx=5)
        self.anim_speed_slider = ttk.Scale(anim_speed_frame,
                                           from_=0, to=100,
                                           orient="horizontal",
                                           command=self.update_anim_speed_label,
                                           style="Glass.Horizontal.TScale")
        self.anim_speed_slider.set(50)
        self.anim_speed_slider.pack(side="left", expand=True, fill="x", padx=5)
        self.anim_speed_label = ttk.Label(anim_speed_frame, text="Normal (50%)", style="Glass.TLabel")
        self.anim_speed_label.pack(side="left", padx=5)
        ttk.Button(anim_speed_frame, text="Apply", command=self.apply_anim_speed, style="Glass.TButton").pack(side="right", padx=5)

        voice_frame = ttk.Frame(self.audio_anim_frame, style="Glass.TFrame")
        voice_frame.pack(fill="x", pady=5)
        ttk.Label(voice_frame, text="Voice Type:", style="Glass.TLabel").pack(side="left", padx=5)
        self.voice_options = ["Standard", "High-pitched", "Deep", "Robotic", "Whisper", "Child-like", "AI Neural"] # Added one
        self.selected_voice = tk.StringVar(value=self.voice_options[0])
        self.voice_combobox = ttk.Combobox(voice_frame,
                                           textvariable=self.selected_voice,
                                           values=self.voice_options,
                                           state="readonly",
                                           style="Glass.TCombobox")
        self.voice_combobox.pack(side="left", expand=True, fill="x", padx=5)
        self.voice_combobox.bind("<<ComboboxSelected>>", self.apply_voice_type)

        mic_sens_frame = ttk.Frame(self.audio_anim_frame, style="Glass.TFrame")
        mic_sens_frame.pack(fill="x", pady=5)
        ttk.Label(mic_sens_frame, text="Mic Sensitivity:", style="Glass.TLabel").pack(side="left", padx=5)
        self.mic_sens_slider = ttk.Scale(mic_sens_frame, from_=0, to=100, orient="horizontal", style="Glass.Horizontal.TScale")
        self.mic_sens_slider.set(60)
        self.mic_sens_slider.pack(side="left", expand=True, fill="x", padx=5)
        self.mic_sens_label = ttk.Label(mic_sens_frame, text="60%", style="Glass.TLabel")
        self.mic_sens_label.pack(side="left", padx=5)
        self.mic_sens_slider.config(command=lambda v: self.mic_sens_label.config(text=f"{int(float(v))}%"))
        ttk.Button(mic_sens_frame, text="Apply", command=lambda: self.show_message("Mic Sensitivity Set", f"Microphone sensitivity set to {int(self.mic_sens_slider.get())}%."), style="Glass.TButton").pack(side="right", padx=5)

        ttk.Button(self.audio_anim_frame, text="Play Custom Sound", command=self.play_custom_sound, style="Glass.TButton").pack(pady=5)
        # New: Gesture Library
        ttk.Button(self.audio_anim_frame, text="Open Gesture Library", command=self.open_gesture_library, style="Glass.TButton").pack(pady=5)
        # New: Play Sound Effect
        sound_effect_frame = ttk.Frame(self.audio_anim_frame, style="Glass.TFrame")
        sound_effect_frame.pack(fill="x", pady=5)
        ttk.Label(sound_effect_frame, text="Sound Effect:", style="Glass.TLabel").pack(side="left", padx=5)
        self.sound_effect_options = ["Chirp", "Boop", "Whistle", "Ding", "Giggle"]
        self.selected_sound_effect = tk.StringVar(value=self.sound_effect_options[0])
        self.sound_effect_combobox = ttk.Combobox(sound_effect_frame,
                                                  textvariable=self.selected_sound_effect,
                                                  values=self.sound_effect_options,
                                                  state="readonly",
                                                  style="Glass.TCombobox")
        self.sound_effect_combobox.pack(side="left", expand=True, fill="x", padx=5)
        ttk.Button(sound_effect_frame, text="Play", command=self.play_sound_effect, style="Glass.TButton").pack(side="right", padx=5)


        # --- Bottom Left: Chat Interface ---
        bottom_left_frame = ttk.Frame(self.main_frame, style="Glass.TFrame")
        bottom_left_frame.grid(row=1, column=0, sticky="nsew", padx=10, pady=10)
        bottom_left_frame.grid_columnconfigure(0, weight=1)
        bottom_left_frame.grid_rowconfigure(0, weight=1)

        ttk.Label(bottom_left_frame, text="Chat Interface", style="Glass.Heading.TLabel").pack(pady=(0, 10))
        self.chat_text = tk.Text(bottom_left_frame, wrap="word", height=8, bg="#2A2A2A", fg=UI_TEXT_COLOR,
                                 insertbackground=UI_ACCENT_COLOR_2, selectbackground=COMPLEMENT_ORANGE_BLUE,
                                 font=("Inter", 10), relief="flat", borderwidth=0)
        self.chat_text.pack(fill="both", expand=True, padx=5, pady=5)
        self.chat_text.insert(tk.END, "Emo Bot: Hello, how may I assist you?\n")
        self.chat_text.config(state=tk.DISABLED) # Make it read-only for display

        chat_input_frame = ttk.Frame(bottom_left_frame, style="Glass.TFrame")
        chat_input_frame.pack(fill="x", pady=5)
        self.chat_entry = ttk.Entry(chat_input_frame, style="Glass.TEntry")
        self.chat_entry.pack(side="left", expand=True, fill="x", padx=5)
        self.chat_entry.bind("<Return>", self.send_chat_message)
        ttk.Button(chat_input_frame, text="Send", command=self.send_chat_message, style="Glass.TButton").pack(side="right", padx=5)


        # --- Bottom Right: System Log ---
        bottom_right_frame = ttk.Frame(self.main_frame, style="Glass.TFrame")
        bottom_right_frame.grid(row=1, column=2, sticky="nsew", padx=10, pady=10)
        bottom_right_frame.grid_columnconfigure(0, weight=1)
        bottom_right_frame.grid_rowconfigure(0, weight=1)

        ttk.Label(bottom_right_frame, text="System Log", style="Glass.Heading.TLabel").pack(pady=(0, 10))
        self.log_text = tk.Text(bottom_right_frame, wrap="word", height=10, bg="#2A2A2A", fg=UI_TEXT_COLOR,
                                insertbackground=UI_ACCENT_COLOR_2, selectbackground=COMPLEMENT_ORANGE_BLUE,
                                font=("Consolas", 9), relief="flat", borderwidth=0) # Monospace font for log
        self.log_text.pack(fill="both", expand=True, padx=5, pady=5)
        self.log_text.tag_config('error', foreground=UI_ERROR_COLOR)
        self.log_text.tag_config('info', foreground=UI_ACCENT_COLOR_2)
        self.log_text.config(state=tk.DISABLED)

        self.mock_errors = [
            "ERROR: Module 'NeuralNet' unresponsive. Retrying...",
            "WARNING: Sensor Array 3 anomaly detected.",
            "CRITICAL: Core Processing Unit overload. Initiating cooldown...",
            "INFO: Battery voltage nominal. Charging initiated.",
            "ERROR: Communication link 'Alpha' degraded. Latency spike detected.",
            "WARNING: Ambient temperature exceeding optimal parameters (35C).",
            "INFO: Firmware checksum verification successful. Version 3.1.2.",
            "ERROR: Vision System calibration failed. Recalibrating...",
            "WARNING: Haptic feedback module intermittently offline.",
            "INFO: Proximity sensor active. Object detected at 0.5m.",
            "ERROR: Audio input buffer overflow. Dropping frames.",
            "WARNING: Task 'Daily Check' overdue by 15 minutes."
        ]
        self.update_log() # Start continuous log updates


        # --- Bottom Bar (Utility & Advanced) ---
        bottom_bar_frame = ttk.Frame(self.main_frame, style="Glass.TFrame")
        bottom_bar_frame.grid(row=2, column=0, columnspan=3, sticky="nsew", padx=10, pady=10) # New row for bottom bar
        bottom_bar_frame.grid_columnconfigure(0, weight=1) # Center buttons
        # Adjusted padding to make it flatter and less like a separate panel
        self.style.configure("Glass.BottomBar.TFrame",
                             background=UI_PANEL_BG,
                             relief="flat",
                             borderwidth=0,
                             padding=(5, 10)) # Less top/bottom padding

        self.utility_frame = ttk.Frame(bottom_bar_frame, style="Glass.BottomBar.TFrame")
        self.utility_frame.pack(pady=5, fill="x", expand=True) # Reduced pady

        ttk.Label(self.utility_frame, text="System Utilities", style="Glass.Heading.TLabel").pack(pady=(0, 5))

        # Buttons for various actions (more organized)
        action_button_row1 = ttk.Frame(self.utility_frame, style="Glass.TFrame")
        action_button_row1.pack(pady=5, fill="x", expand=True)
        ttk.Button(action_button_row1, text="Send Custom Command", command=self.send_custom_command, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row1, text="Open Task Scheduler", command=self.open_task_scheduler, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row1, text="Check for Updates", command=self.check_for_updates, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row1, text="Toggle Remote Control", command=self.toggle_remote_control, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row1, text="Toggle Data Logging", command=self.toggle_data_logging, style="Glass.TButton").pack(side="left", padx=5, pady=5)

        action_button_row2 = ttk.Frame(self.utility_frame, style="Glass.TFrame")
        action_button_row2.pack(pady=5, fill="x", expand=True)
        # New Buttons
        ttk.Button(action_button_row2, text="Diagnostic Report", command=self.generate_diagnostic_report, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row2, text="Memory Management", command=self.open_memory_manager, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row2, text="Backup/Restore Settings", command=self.backup_restore_settings, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row2, text="Set Alarm", command=self.set_alarm, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row2, text="Run Self-Test", command=self.run_self_test, style="Glass.TButton").pack(side="left", padx=5, pady=5) # 9th new button

        action_button_row3 = ttk.Frame(self.utility_frame, style="Glass.TFrame") # For reset/restart
        action_button_row3.pack(pady=5, fill="x", expand=True)
        ttk.Button(action_button_row3, text="Reset Bot to Default", command=self.reset_bot, style="Glass.TButton").pack(side="left", padx=5, pady=5)
        ttk.Button(action_button_row3, text="Restart Bot Software", command=self.restart_bot_software, style="Glass.TButton").pack(side="left", padx=5, pady=5)

        # New: Software Version Info
        self.version_info_label = ttk.Label(self.utility_frame, text="Firmware: v3.1.2 | OS: EMO-OS v1.5.0 | Last Update: 2025.06.25", style="Glass.Small.TLabel")
        self.version_info_label.pack(pady=(10, 0))


    # --- GIF Animation Methods ---
    def load_gif_frames(self):
        try:
            image = Image.open(self.gif_path)
            self.gif_frames = []
            for frame_num in range(0, image.n_frames):
                image.seek(frame_num)
                # Resize image to fit canvas while maintaining aspect ratio
                canvas_width = self.model_canvas.winfo_width() if self.model_canvas.winfo_width() > 1 else 300
                canvas_height = self.model_canvas.winfo_height() if self.model_canvas.winfo_height() > 1 else 300
                img_width, img_height = image.size
                aspect_ratio = img_width / img_height

                if canvas_width / canvas_height > aspect_ratio:
                    new_height = canvas_height
                    new_width = int(canvas_height * aspect_ratio)
                else:
                    new_width = canvas_width
                    new_height = int(canvas_width / aspect_ratio)

                resized_frame = image.resize((new_width, new_height), Image.Resampling.LANCZOS)
                self.gif_frames.append(ImageTk.PhotoImage(resized_frame))
        except Exception as e:
            print(f"Error loading GIF: {e}")
            self.gif_frames = [] # Clear frames if error occurs

    def animate_gif(self):
        if self.gif_frames:
            # Update image on canvas
            self.model_canvas.itemconfig(self.gif_image_on_canvas, image=self.gif_frames[self.gif_frame_idx])
            # Center the image
            self.model_canvas.coords(
                self.gif_image_on_canvas,
                self.model_canvas.winfo_width() / 2,
                self.model_canvas.winfo_height() / 2
            )

            self.gif_frame_idx = (self.gif_frame_idx + 1) % len(self.gif_frames)
            self.master.after(self.gif_delay, self.animate_gif)
        else:
            # If GIF not loaded, display placeholder text more robustly
            if not self.model_canvas.find_withtag("gif_image"):
                self.model_canvas.delete("all")
                self.model_canvas.create_text(
                    self.model_canvas.winfo_width() / 2, self.model_canvas.winfo_height() / 2 - 20,
                    text="animatron.gif not found!", fill=UI_ERROR_COLOR, font=("Inter", 12, "bold")
                )
                self.model_canvas.create_text(
                    self.model_canvas.winfo_width() / 2, self.model_canvas.winfo_height() / 2 + 10,
                    text="Please place 'animatron.gif' in the same directory.", fill=UI_LIGHT_TEXT_COLOR, font=("Inter", 10)
                )

    def _resize_canvas_bg(self, event):
        # Update the rounded rectangle to fill the canvas on resize
        self.model_canvas.coords(self.canvas_bg_rect, 0, 0, event.width, event.height)
        # Reload GIF frames on resize to adjust their size
        if self.gif_frames:
            try:
                self.load_gif_frames() # Reload to resize images
                # Ensure the image is updated immediately after resize
                self.model_canvas.itemconfig(self.gif_image_on_canvas, image=self.gif_frames[self.gif_frame_idx])
                self.model_canvas.coords(
                    self.gif_image_on_canvas,
                    self.model_canvas.winfo_width() / 2,
                    self.model_canvas.winfo_height() / 2
                )
            except Exception as e:
                print(f"Error during GIF resize: {e}")

    # --- Log Update Methods ---
    def update_log(self):
        current_time = time.strftime("[%H:%M:%S]")
        message = random.choice(self.mock_errors)
        tag = 'error' if "ERROR" in message or "CRITICAL" in message else 'info'

        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, f"{current_time} {message}\n", tag)
        self.log_text.see(tk.END) # Scroll to the end
        if int(self.log_text.index('end-1c').split('.')[0]) > 50: # Keep log to 50 lines
            self.log_text.delete('1.0', '2.0')
        self.log_text.config(state=tk.DISABLED)
        self.master.after(random.randint(1000, 3000), self.update_log) # Update every 1-3 seconds

    # --- Chat Methods ---
    def send_chat_message(self, event=None):
        user_message = self.chat_entry.get()
        if user_message.strip():
            self.chat_text.config(state=tk.NORMAL)
            self.chat_text.insert(tk.END, f"You: {user_message}\n", 'user_msg')
            self.chat_text.see(tk.END)
            self.chat_entry.delete(0, tk.END)
            self.chat_text.config(state=tk.DISABLED)
            # Simulate bot response
            self.master.after(500, lambda: self.simulate_bot_response(user_message))

    def simulate_bot_response(self, user_msg):
        responses = [
            "Processing your request...",
            "Understood. Analyzing data.",
            "Affirmative. Command executed.",
            "Query received. Awaiting further instruction.",
            "Acknowledged. Initiating response protocol."
        ]
        bot_response = random.choice(responses)
        self.chat_text.config(state=tk.NORMAL)
        self.chat_text.insert(tk.END, f"Emo Bot: {bot_response}\n", 'bot_msg')
        self.chat_text.see(tk.END)
        self.chat_text.config(state=tk.DISABLED)

    # --- Bot Control Functions (Placeholders) ---
    def connect_bot(self):
        print("Attempting to connect to Emo Bot...")
        self.status_label.config(text="Connection: Connected", foreground=COMPLEMENT_VIOLET_LIME) # Green for connected
        self.show_message("Bot Connected!", "The Emo Bot is now connected. Online and operational!")

    def disconnect_bot(self):
        print("Disconnecting from Emo Bot...")
        self.status_label.config(text="Connection: Disconnected", foreground=COMPLEMENT_CYAN_RED) # Red for disconnected
        self.show_message("Bot Disconnected!", "The Emo Bot has been disconnected. Powering down systems.")

    def apply_wifi_settings(self):
        ssid = self.wifi_ssid_entry.get()
        password = self.wifi_password_entry.get()
        print(f"Applying WiFi settings: SSID='{ssid}', Password='{password}'")
        self.show_message("WiFi Configured", f"Attempting to connect to WiFi network: '{ssid}'.")

    def pair_bluetooth(self):
        print("Initiating Bluetooth pairing...")
        self.show_message("Bluetooth Pairing", "Scanning for Bluetooth devices... Please put your device in pairing mode.")

    def check_battery(self):
        # Simulate battery level change
        new_level = random.randint(30, 99)
        self.battery_label.config(text=f"Battery: {new_level}%")
        print(f"Checking battery: {new_level}%")
        self.show_message("Battery Status", f"Current battery level: {new_level}%.")

    def set_bot_name(self):
        name = self.bot_name_entry.get()
        print(f"Setting bot name to: {name}")
        self.show_message("Name Set", f"Bot designation updated to '{name}'. Unit will respond accordingly.")

    def pick_eye_color(self):
        color_code = colorchooser.askcolor(title="Choose Eye/LED Color", initialcolor=self.current_eye_color.get())
        if color_code[1]: # color_code[1] is the hex string if a color was chosen
            self.current_eye_color.set(color_code[1])
            self.eye_color_display.config(bg=color_code[1])
            print(f"Picked color: {color_code[1]}")

    def apply_eye_color(self):
        color = self.current_eye_color.get()
        print(f"Applying eye/LED color: {color}")
        self.show_message("Color Applied", f"Optical sensors and LED array calibrated to {color}. Looks sharp!")

    def apply_accessory(self, event=None):
        accessory = self.selected_accessory.get()
        print(f"Applying accessory: {accessory}")
        self.show_message("Accessory Set", f"Physical accessory '{accessory}' activated. Fashion protocols engaged.")

    def apply_material(self, event=None):
        material = self.selected_material.get()
        print(f"Applying body material: {material}")
        self.show_message("Material Set", f"Bot's exterior shell material updated to '{material}'.")

    def open_light_editor(self):
        print("Opening light pattern editor (simulated)...")
        self.show_message("Light Editor", "Opening advanced light pattern customization interface.")

    def update_mood_label(self, value):
        mood_val = int(float(value))
        if mood_val < 25:
            mood_text = "Sad"
        elif mood_val < 50:
            mood_text = "Neutral"
        elif mood_val < 75:
            mood_text = "Happy"
        else:
            mood_text = "Mischievous"
        self.mood_label.config(text=f"{mood_text} ({mood_val}%)")

    def apply_mood(self):
        mood_value = int(self.mood_slider.get())
        print(f"Setting bot mood to: {mood_value}%")
        self.show_message("Mood Set", f"Emotional subroutine adjusted to {self.mood_label['text'].split(' ')[0]}.")

    def apply_interaction_style(self):
        style = self.selected_interaction.get()
        print(f"Setting interaction style to: {style}")
        self.show_message("Interaction Set", f"Interaction matrix recalibrated to '{style}' mode.")

    def toggle_sleep_mode(self):
        status = "enabled" if self.sleep_mode_var.get() else "disabled"
        print(f"Sleep mode is now: {status}")
        self.show_message("Sleep Mode", f"Sleep mode is now {status}. Unit conserving power.")

    def toggle_follow_gaze(self):
        status = "enabled" if self.follow_gaze_var.get() else "disabled"
        print(f"Follow gaze is now: {status}")
        self.show_message("Gaze Tracking", f"Gaze tracking is now {status}. Unit will follow your movements.")

    def toggle_learning_mode(self):
        status = "enabled" if self.learning_mode_var.get() else "disabled"
        print(f"Learning mode is now: {status}")
        self.show_message("Learning Mode", f"Adaptive learning protocols are now {status}.")

    def toggle_auto_respond(self):
        status = "enabled" if self.auto_respond_var.get() else "disabled"
        print(f"Auto-respond is now: {status}")
        self.show_message("Auto-Respond", f"Auto-response protocol is now {status}.")

    def update_volume_label(self, value):
        volume_val = int(float(value))
        self.volume_label.config(text=f"{volume_val}%")

    def apply_volume(self):
        volume = int(self.volume_slider.get())
        print(f"Setting bot volume to: {volume}%")
        self.show_message("Volume Set", f"Audio output configured to {volume}%.")

    def update_anim_speed_label(self, value):
        speed_val = int(float(value))
        if speed_val < 25:
            speed_text = "Slow"
        elif speed_val < 75:
            speed_text = "Normal"
        else:
            speed_text = "Fast"
        self.anim_speed_label.config(text=f"{speed_text} ({speed_val}%)")

    def apply_anim_speed(self):
        speed = int(self.anim_speed_slider.get())
        print(f"Setting animation speed to: {speed}%")
        self.show_message("Speed Set", f"Movement kinetics adjusted to {self.anim_speed_label['text'].split(' ')[0]} speed.")

    def apply_voice_type(self, event=None):
        voice_type = self.selected_voice.get()
        print(f"Setting voice type to: {voice_type}")
        self.show_message("Voice Set", f"Vocal modulator set to '{voice_type}' timbre.")

    def play_custom_sound(self):
        print("Playing a custom sound (simulated)...")
        self.show_message("Sound Played", "A custom audio sequence has been initiated.")

    def open_gesture_library(self):
        print("Opening gesture library (simulated)...")
        self.show_message("Gesture Library", "Accessing Emo Bot's gestural repertoire.")

    def play_sound_effect(self):
        effect = self.selected_sound_effect.get()
        print(f"Playing sound effect: {effect}")
        self.show_message("Sound Effect", f"Playing sound effect: '{effect}'.")

    def send_custom_command(self):
        # Create a Toplevel window for custom command input
        command_window = tk.Toplevel(self.master)
        command_window.title("Send Custom Command")
        command_window.geometry("400x200")
        command_window.transient(self.master)
        command_window.grab_set()
        command_window.configure(bg=UI_PANEL_BG)

        ttk.Label(command_window, text="Enter Command:", style="Glass.TLabel").pack(pady=10)
        command_entry = ttk.Entry(command_window, style="Glass.TEntry", width=40)
        command_entry.pack(pady=5)
        command_entry.insert(0, "diagnose_subsystem_alpha")

        def execute_command():
            cmd = command_entry.get()
            print(f"Executing custom command: {cmd}")
            self.show_message("Command Sent", f"Command '{cmd}' dispatched to Emo Bot.")
            command_window.destroy()

        ttk.Button(command_window, text="Execute", command=execute_command, style="Glass.TButton").pack(pady=10)
        command_window.wait_window(command_window)

    def open_task_scheduler(self):
        # Create a new Toplevel window for the task scheduler
        scheduler_window = tk.Toplevel(self.master)
        scheduler_window.title("Emo Bot Task Scheduler")
        scheduler_window.geometry("500x400")
        scheduler_window.transient(self.master)
        scheduler_window.grab_set()

        scheduler_window.configure(bg=UI_PANEL_BG) # Use panel background for consistency

        ttk.Label(scheduler_window, text="Scheduled Tasks (Simulated)", style="Glass.Heading.TLabel").pack(pady=15)

        ttk.Label(scheduler_window, text="Task Name:", style="Glass.TLabel").pack(anchor="w", padx=10, pady=2)
        task_name_entry = ttk.Entry(scheduler_window, style="Glass.TEntry")
        task_name_entry.pack(fill="x", padx=10, pady=2)
        task_name_entry.insert(0, "Perform Daily Check")

        ttk.Label(scheduler_window, text="Schedule (e.g., 'Daily 8AM', 'Weekly Mon 10PM'):", style="Glass.TLabel").pack(anchor="w", padx=10, pady=2)
        task_schedule_entry = ttk.Entry(scheduler_window, style="Glass.TEntry")
        task_schedule_entry.pack(fill="x", padx=10, pady=2)
        task_schedule_entry.insert(0, "Daily 09:00")

        ttk.Button(scheduler_window, text="Add Task", style="Glass.TButton",
                   command=lambda: self.show_message("Task Added", f"Task '{task_name_entry.get()}' scheduled for '{task_schedule_entry.get()}'")).pack(pady=15)

        ttk.Label(scheduler_window, text="Current Tasks:", style="Glass.SubHeading.TLabel").pack(pady=(10, 5))
        ttk.Label(scheduler_window, text="- Greet User (Daily 7AM)\n- Check Weather (Hourly)\n- Update Firmware (Weekly Sunday)", style="Glass.Small.TLabel").pack(anchor="w", padx=10)

        scheduler_window.wait_window(scheduler_window) # Wait for this window to close

    def check_for_updates(self):
        print("Checking for bot software updates...")
        self.show_message("Software Update", "Checking for latest firmware... No updates available at this time.")

    def toggle_remote_control(self):
        current_state = "enabled" if not hasattr(self, '_remote_control_enabled') or not self._remote_control_enabled else "disabled"
        self._remote_control_enabled = not hasattr(self, '_remote_control_enabled') or not self._remote_control_enabled
        print(f"Remote control is now: {current_state}")
        self.show_message("Remote Control", f"Remote control access is now {current_state}.")

    def toggle_data_logging(self):
        current_state = "enabled" if not hasattr(self, '_data_logging_enabled') or not self._data_logging_enabled else "disabled"
        self._data_logging_enabled = not hasattr(self, '_data_logging_enabled') or not self._data_logging_enabled
        print(f"Data logging is now: {current_state}")
        self.show_message("Data Logging", f"Performance data logging is now {current_state}.")

    def generate_diagnostic_report(self):
        print("Generating diagnostic report...")
        self.show_message("Diagnostic Report", "Compiling system diagnostics. Report will be saved to logs.")

    def open_memory_manager(self):
        print("Opening memory manager (simulated)...")
        self.show_message("Memory Manager", "Accessing Emo Bot's memory allocation and optimization tools.")

    def backup_restore_settings(self):
        print("Opening backup/restore dialog (simulated)...")
        self.show_message("Backup/Restore", "Initiating settings backup and restore utility.")

    def set_alarm(self):
        alarm_window = tk.Toplevel(self.master)
        alarm_window.title("Set Emo Bot Alarm")
        alarm_window.geometry("350x200")
        alarm_window.transient(self.master)
        alarm_window.grab_set()
        alarm_window.configure(bg=UI_PANEL_BG)

        ttk.Label(alarm_window, text="Alarm Time (HH:MM):", style="Glass.TLabel").pack(pady=10)
        alarm_time_entry = ttk.Entry(alarm_window, style="Glass.TEntry")
        alarm_time_entry.pack(pady=5)
        alarm_time_entry.insert(0, "07:30")

        def add_alarm():
            alarm_time = alarm_time_entry.get()
            print(f"Setting alarm for: {alarm_time}")
            self.show_message("Alarm Set", f"Alarm activated for Emo Bot at {alarm_time}.")
            alarm_window.destroy()

        ttk.Button(alarm_window, text="Set Alarm", command=add_alarm, style="Glass.TButton").pack(pady=10)
        alarm_window.wait_window(alarm_window)

    def run_self_test(self):
        print("Initiating bot self-test sequence...")
        self.show_message("Self-Test", "Emo Bot is running a comprehensive system diagnostic self-test. Please wait.")


    def reset_bot(self):
        print("Resetting bot to default settings...")
        self.show_message("Bot Reset", "The Emo Bot has been reset to factory defaults. Rebooting parameters.")
        # Reset all UI controls to their initial values
        self.bot_name_entry.delete(0, tk.END)
        self.bot_name_entry.insert(0, "A.K.I.R.A Unit 7")
        self.current_eye_color.set(COLOR_CYAN)
        self.eye_color_display.config(bg=COLOR_CYAN)
        self.selected_accessory.set(self.accessory_options[0])
        self.selected_material.set(self.material_options[0])
        self.mood_slider.set(50)
        self.update_mood_label("50")
        self.selected_interaction.set(self.interaction_options[2])
        self.auto_respond_var.set(True)
        self.sleep_mode_var.set(False)
        self.follow_gaze_var.set(True)
        self.learning_mode_var.set(True)
        self.expressiveness_slider.set(70)
        self.expressiveness_label.config(text="70%")
        self.space_sens_slider.set(50)
        self.space_sens_label.config(text="50%")
        self.volume_slider.set(75)
        self.update_volume_label("75")
        self.anim_speed_slider.set(50)
        self.update_anim_speed_label("50")
        self.selected_voice.set(self.voice_options[0])
        self.mic_sens_slider.set(60)
        self.mic_sens_label.config(text="60%")
        self.selected_sound_effect.set(self.sound_effect_options[0])

    def restart_bot_software(self):
        print("Restarting bot software...")
        self.show_message("Bot Restarted", "The Emo Bot operating system is initiating restart sequence.")

    # Custom message box for feedback (replaces alert/confirm)
    def show_message(self, title, message):
        top = tk.Toplevel(self.master)
        top.title(title)
        top.geometry("380x220") # Larger message box
        top.transient(self.master) # Make sure it stays on top of the main window
        top.grab_set() # Disable interaction with the main window until this is closed

        # Style for the message box
        top.configure(bg=UI_PANEL_BG)
        ttk.Label(top, text=message, style="Glass.TLabel", wraplength=350, font=("Inter", 12)).pack(pady=25)
        ttk.Button(top, text="OK", command=top.destroy, style="Glass.TButton").pack(pady=15)
        self.master.wait_window(top) # Wait for the Toplevel window to close

if __name__ == "__main__":
    root = tk.Tk()
    app = EmoBotControllerApp(root)
    root.mainloop()
