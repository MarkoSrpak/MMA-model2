"""
interactive_dashboard.py
------------------------
Dash aplikacija koja dinamički prikazuje rutu, sve numeričke grafove 
i fitness analitiku. Kontrole (gender, age, weight, height) su sada
**Dropdown** / **Radio** sučelje, a analiza sadrži BMI, MET, procjenu
kalorija **i prosječni AQI**.

Dodatno:
- Karta rute prikazuje segmentiranu liniju u skali **crvena–žuta–zelena**
  (Plotly skala `RdYlGn`) ovisno o prosječnoj brzini svakih 100 m.
- Tamna pozadina karti i grafovi se slažu s cijelim dark‑mode izgledom.

Pokreni:  python interactive_dashboard.py
Zavisnosti:  pip install dash pandas plotly numpy
"""

from pathlib import Path

import numpy as np
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from plotly.colors import sample_colorscale
from dash import Dash, html, dcc, Input, Output

# ---------------------------------------------------------------------------
# 1. POSTAVKE / UČITAVANJE PODATAKA
# ---------------------------------------------------------------------------
from pathlib import Path
import pandas as pd

DATA_PATH = Path("data/WO663.txt")
SEP = ","

NEW_COLS = [
    "ts_ms", "year", "mon", "day", "hr", "min", "sec",
    "latitude", "longitude", "speed", "altitude",
    "temperature", "pressure", "humidity", "voc_ohm",
    "noise", "sweat", "accel_freq", "accel_ampl", "accel_energy", "accel_dir"
]



# Učitavanje podataka (preskače header redak)
df = pd.read_csv(DATA_PATH, sep=SEP, names=NEW_COLS, skiprows=1)

# Izbaci redove gdje su GPS koordinate (0, 0)
df = df[~((df["latitude"] == 0.0) & (df["longitude"] == 0.0))].copy()

# Preimenovanje kolona za kompatibilnost s pd.to_datetime i ostatkom koda
df.rename(columns={
    "mon": "month",
    "hr": "hour",
    "min": "minute",
    "sec": "second",
    "air_quality": "air_quality"
}, inplace=True)

# Parsiraj stupce za vrijeme u numerički format (ako već nisu)
for col in ["year", "month", "day", "hour", "minute", "second"]:
    df[col] = pd.to_numeric(df[col], errors="coerce")

# Ukloni redove koji imaju NaN u vremenskim stupcima
df.dropna(subset=["year", "month", "day", "hour", "minute", "second"], inplace=True)

# Kreiraj 'time' kolonu
df["time"] = pd.to_datetime(df[["year", "month", "day", "hour", "minute", "second"]])


# Konverzija numeričkih kolona osim onih koje ne želimo u grafovima
num_cols = [
    c for c in df.columns if c not in [
        "time", "longitude", "latitude", "year", "month", "day", "hour", "minute", "second", "ts_ms", "accel_dir"
    ]
]
df[num_cols] = df[num_cols].apply(pd.to_numeric, errors="coerce")

def energy_to_db(energy, ref_energy=0.1):
    """Pretvara energiju mikrofona u decibele u odnosu na referentnu razinu."""
    energy = np.maximum(energy, 1e-9)  # zaštita od log(0)
    return 10 * np.log10(energy / ref_energy)

df["temperature"] = df["temperature"] / 100.0
df["humidity"] = df["humidity"] / 1000.0
df["noise"] = np.where(df["noise"] > 100000, 0, df["noise"])
df["noise"] = energy_to_db(df["noise"], ref_energy=0.1)
df["sweat"] = np.minimum(df["sweat"], 4096)

pretty_names = {
    "speed": "Speed (m/s)",
    "altitude": "Altitude (m)",
    "temperature": "Temperature (°C)",
    "pressure": "Pressure (hPa)",
    "humidity": "Humidity (%)",
    "voc_aqi": "Air Quality Index (AQI)",
    "noise": "Noise Level",
    "sweat": "Sweat Level",
    "accel_freq": "Acceleration Frequency",
    "accel_ampl": "Acceleration Amplitude",
    "accel_energy": "Acceleration Energy",
}

# ---------------------------------------------------------------------------
# 1.a PRETVORBA 'air quality' U REALNI AQI (EPA skala)
# ---------------------------------------------------------------------------
def bme680_voc_to_aqi(voc_ohm: float, voc_min=100_000, voc_max=750_000) -> float:
    """
    Inverzna normalizacija VOC otpora iz BME680 u AQI-ličnu vrijednost (0–500).
    Viši otpor = čišći zrak.
    """
    voc_ohm = np.clip(voc_ohm, voc_min, voc_max)
    norm = (voc_ohm - voc_min) / (voc_max - voc_min)
    aqi = 500 * (1 - norm)  # invertirano: manji otpor = veći AQI
    return aqi


df["voc_aqi"] = df["voc_ohm"].apply(bme680_voc_to_aqi)
num_cols.append("voc_aqi")


#def pm25_to_aqi(pm: float) -> float:
#    """Gruba pretvorba PM2.5 koncentracije (µg/m³) u AQI prema EPA break‑pointima."""
#    breakpoints = [
#        (0.0, 12.0, 0, 50),
#        (12.1, 35.4, 51, 100),
#        (35.5, 55.4, 101, 150),
#        (55.5, 150.4, 151, 200),
#        (150.5, 250.4, 201, 300),
#        (250.5, 500.4, 301, 500),
#    ]
#    for c_low, c_high, aqi_low, aqi_high in breakpoints:
#        if c_low <= pm <= c_high:
#            return aqi_low + (pm - c_low) * (aqi_high - aqi_low) / (c_high - c_low)
#    return 500.0  # izvan opsega – saturiraj na max

#num_cols.append("voc_aqi")
# Zamijeni sirove vrijednosti AQI skaliranim (pretpostavka: kolona sadrži PM2.5 µg/m³)
#df["air quality"] = df["air quality"].apply(pm25_to_aqi)

# ---------------------------------------------------------------------------
# 2. GEO‑POMOĆNE FUNKCIJE
# ---------------------------------------------------------------------------

def haversine_vec(lat1, lon1, lat2, lon2):
    """Vraća udaljenost (m) između dvije točke pomoću haversine formule."""
    R = 6_371_000  # polumjer Zemlje u metrima
    dlat = np.radians(lat2 - lat1)
    dlon = np.radians(lon2 - lon1)
    a = np.sin(dlat / 2) ** 2 + np.cos(np.radians(lat1)) * np.cos(np.radians(lat2)) * np.sin(dlon / 2) ** 2
    c = 2 * np.arcsin(np.sqrt(a))
    return R * c


def build_segments(df_in: pd.DataFrame, segment_len_m: int = 100) -> pd.DataFrame:
    """Dodaje kolone za kumulativnu udaljenost i segmentni ID od segment_len_m metara."""
    lat = df_in["latitude"].to_numpy()
    lon = df_in["longitude"].to_numpy()

    # udaljenosti između uzastopnih točaka
    dist = np.zeros_like(lat)
    dist[1:] = haversine_vec(lat[:-1], lon[:-1], lat[1:], lon[1:])

    df_out = df_in.copy()
    df_out["seg_dist"] = dist
    df_out["cum_dist"] = dist.cumsum()
    df_out["seg_id"] = (df_out["cum_dist"] // segment_len_m).astype(int)
    return df_out


df = build_segments(df, 100)

# ---------------------------------------------------------------------------
# 3. FITNES POMOĆNA LOGIKA
# ---------------------------------------------------------------------------

def speed_to_met(speed_kmh: float) -> float:
    """Vrati MET vrijednost ovisno o prosj. brzini bicikliranja."""
    if speed_kmh < 16:
        return 4.0  # lagano
    elif speed_kmh < 19:
        return 6.0  # umjereno
    elif speed_kmh < 22:
        return 8.0  # brzo
    elif speed_kmh < 25:
        return 10.0  # vrlo brzo
    else:
        return 12.0  # natjecateljsko

# ---------------------------------------------------------------------------
# 4. FUNKCIJA ZA KARTU RUTE
# ---------------------------------------------------------------------------

def make_geo_figure() -> "plotly.graph_objs._figure.Figure":
    """Generira mapbox figuru sa segmentiranom rutom obojanom po brzini (gradijentna linija)."""
    lat_min, lat_max = df["latitude"].min(), df["latitude"].max()
    lon_min, lon_max = df["longitude"].min(), df["longitude"].max()
    center = {"lat": (lat_min + lat_max) / 2, "lon": (lon_min + lon_max) / 2}

    # Odredi zoom
    lat_range = lat_max - lat_min
    lon_range = lon_max - lon_min
    max_range = max(lat_range, lon_range)
    zoom = 11
    if max_range < 0.5:
        zoom = 12
    if max_range < 0.1:
        zoom = 13
    if max_range < 0.05:
        zoom = 14
    if max_range < 0.01:
        zoom = 15
    if max_range < 0.005:
        zoom = 16

    # Normalizacija brzine
    min_speed = df["speed"].min()
    max_speed = df["speed"].max() or 1
    speed_norm = (df["speed"] - min_speed) / (max_speed - min_speed)
    color_values = speed_norm.to_numpy()

    segments = []
    for i in range(len(df) - 1):
        segments.append(go.Scattermapbox(
            lat=[df.iloc[i]["latitude"], df.iloc[i + 1]["latitude"]],
            lon=[df.iloc[i]["longitude"], df.iloc[i + 1]["longitude"]],
            mode="lines",
            line=dict(
                width=4,
                color=sample_colorscale("BlueRed", color_values[i])[0],
            ),
            hoverinfo="text",
            text=[f"{df.iloc[i]['time']}<br>Speed: {df.iloc[i]['speed']:.1f} km/h"],
            showlegend=False,
        ))

    fig = go.Figure(segments)

    fig.update_layout(
        mapbox_style="carto-positron",
        mapbox_center=center,
        mapbox_zoom=zoom,
        margin=dict(l=0, r=0, t=40, b=0),
        template="plotly_dark",
        height=500,
    )
    return fig


# ---------------------------------------------------------------------------
# 5. GRAFS  METRICS (ostaje isto – sada uključuje skalirani 'air quality')
# ---------------------------------------------------------------------------
metric_figs = []
for col in num_cols:
    title = pretty_names.get(col, col.replace("_", " ").capitalize())
    fig = px.line(df, x="time", y=col, title=title, template="plotly_dark", height=350)
    fig.update_layout(margin=dict(l=40, r=40, t=60, b=40), hovermode="x unified")
    
    if col == "humidity":
        fig.update_yaxes(ticksuffix=" %", range=[0, 100])
    
    metric_figs.append(fig)

# ---------------------------------------------------------------------------
# 6. DASH APLIKACIJA
# ---------------------------------------------------------------------------
app = Dash(
    __name__,
    external_stylesheets=["https://fonts.googleapis.com/css2?family=Raleway:wght@400;600&display=swap"],
)
server = app.server
app.title = "MMA MODEL 2"

label_style = {"marginTop": "20px", "display": "block"}

app.layout = html.Div(
    style={"background": "#2e2e2e", "color": "#ddd", "fontFamily": "Raleway, sans-serif", "padding": "20px"},
    children=[
        html.H1("MMA MODEL 2", style={"textAlign": "center", "marginTop": 0}),
        html.H2("Workout & Performance Analysis", style={"textAlign": "center", "color": "#ccc", "marginTop": "-10px"}),
        html.Hr(),
        # ---------------- Kontrole ----------------
        html.Div(
            style={"display": "flex", "flexWrap": "wrap", "gap": "40px", "justifyContent": "center"},
            children=[
                html.Div([
                    html.Label("Gender", style=label_style),
                    dcc.RadioItems(
                        id="gender",
                        options=[{"label": "Female", "value": "F"}, {"label": "Male", "value": "M"}],
                        value="F",
                        labelStyle={"display": "inline-block", "marginRight": "15px"},
                        inputStyle={"marginRight": "6px"},
                        style={"color": "#ddd"},
                    ),
                ]),
                html.Div([
                    html.Label("Age", style=label_style),
                    dcc.Dropdown(id="age", options=[{"label": str(a), "value": a} for a in range(2, 99)], value=30, clearable=False),
                ]),
                html.Div([
                    html.Label("Weight (kg)", style=label_style),
                    dcc.Dropdown(id="weight", options=[{"label": str(w), "value": w} for w in range(30, 200)], value=70, clearable=False),
                ]),
                html.Div([
                    html.Label("Height (cm)", style=label_style),
                    dcc.Dropdown(id="height", options=[{"label": str(h), "value": h} for h in range(100, 211)], value=170, clearable=False),
                ]),
            ],
        ),
        html.Br(),
        # --------------- Analiza ------------------
        html.Div(id="analysis", style={"fontSize": "18px", "lineHeight": "1.6", "marginBottom": "30px"}),
        # --------------- Karta   ------------------
        dcc.Graph(id="geo", figure=make_geo_figure(), config={"displaylogo": False}),
        html.Br(),
        # --------------- Grid grafovi -------------
        html.Div(
            [dcc.Graph(figure=fig, config={"displaylogo": False}) for fig in metric_figs],
            style={"display": "grid", "gridTemplateColumns": "repeat(auto-fit, minmax(500px, 1fr))", "gap": "30px"},
        ),
    ],
)

# ---------------------------------------------------------------------------
# 7. CALLBACK – LIVE ANALIZA
# ---------------------------------------------------------------------------
@app.callback(
    Output("analysis", "children"),
    Input("gender", "value"),
    Input("age", "value"),
    Input("weight", "value"),
    Input("height", "value"),
)

def update_analysis(gender: str, age: int, weight: float, height: float):
    total_seconds = (df["time"].iloc[-1] - df["time"].iloc[0]).total_seconds()
    hours = total_seconds / 3600
    time_deltas = df["time"].diff().dt.total_seconds().fillna(0)
    distance_km = (df["speed"] ).sum()/1000
    avg_speed = df["speed"].mean()*3.6
    altitude_change = df["altitude"].max() - df["altitude"].min()

    # --- kalorije ---
    met = speed_to_met(avg_speed)
    calories = met * weight * hours

    # --- BMI ---
    bmi = weight / ((height / 100) ** 2)
    if bmi < 18.5:
        bmi_category, bmi_color = "Underweight", "lightblue"
    elif bmi < 25:
        bmi_category, bmi_color = "Normal", "green"
    elif bmi < 30:
        bmi_category, bmi_color = "Overweight", "orange"
    else:
        bmi_category, bmi_color = "Obese", "red"

    # --- AQI ---
    aqi = df["voc_aqi"].mean()
    if aqi <= 50:
        aqi_color = "green"
    elif aqi <= 100:
        aqi_color = "yellow"
    elif aqi <= 150:
        aqi_color = "orange"
    elif aqi <= 200:
        aqi_color = "red"
    elif aqi <= 300:
        aqi_color = "purple"
    else:
        aqi_color = "maroon"

    # --- BUČNOST ---
    valid_noise = df[df["noise"] <= 100000]["noise"]
    avg_noise = valid_noise.mean()
    noise_warning = ""
    if avg_noise > 85:
        noise_warning = f"Average noise level is {avg_noise:.1f} dB — this may be harmful over time!"
    elif avg_noise > 70:
        noise_warning = f"ℹNoise level is {avg_noise:.1f} dB — consider ear protection if exposure is prolonged."

    # --- Gauge BMI ---
    fig_bmi = go.Figure(go.Indicator(
        mode="gauge+number",
        value=bmi,
        title={"text": "BMI", "font": {"size": 24, "color": "#ddd"}},
        gauge={
            "axis": {"range": [10, 40], "tickcolor": "#ddd", "tickfont": {"color": "#ddd"}},
            "bar": {"color": bmi_color},
            "steps": [
                {"range": [10, 18.5], "color": "lightblue"},
                {"range": [18.5, 25], "color": "lightgreen"},
                {"range": [25, 30], "color": "orange"},
                {"range": [30, 40], "color": "red"},
            ],
            "threshold": {"line": {"color": "black", "width": 4}, "thickness": 0.75, "value": bmi},
        },
    ))
    fig_bmi.update_layout(margin=dict(l=0, r=0, t=40, b=0), height=250, plot_bgcolor="rgba(0,0,0,0)", paper_bgcolor="rgba(0,0,0,0)", font={"color": "#ddd"})

    # --- Gauge AQI ---
    fig_aqi = go.Figure(go.Indicator(
        mode="gauge+number",
        value=aqi,
        title={"text": "Air Quality Index", "font": {"size": 24, "color": "#ddd"}},
        gauge={
            "axis": {"range": [0, 500], "tickcolor": "#ddd", "tickfont": {"color": "#ddd"}},
            "bar": {"color": aqi_color},
            "steps": [
                {"range": [0, 50], "color": "green"},
                {"range": [50, 100], "color": "yellow"},
                {"range": [100, 150], "color": "orange"},
                {"range": [150, 200], "color": "red"},
                {"range": [200, 300], "color": "purple"},
                {"range": [300, 500], "color": "maroon"},
            ],
            "threshold": {"line": {"color": "black", "width": 4}, "thickness": 0.75, "value": aqi},
        },
    ))
    fig_aqi.update_layout(margin=dict(l=0, r=0, t=40, b=0), height=250, plot_bgcolor="rgba(0,0,0,0)", paper_bgcolor="rgba(0,0,0,0)", font={"color": "#ddd"})

    # --- Tekst ---
    text_items = [
        html.P(f"BMI = {bmi:.1f} ({bmi_category})"),
        html.P(f"Average Speed: {avg_speed:.2f} km/h"),
        html.P(f"MET (metabolic equivalent of task) ≈ {met}"),
        html.P(f"Distance Covered: {distance_km:.2f} km"),
        html.P(f"Duration: {hours * 60:.1f} min"),
        html.P(f"Altitude Change: {altitude_change:.2f} m"),
        html.P(f"Calories Burned (est.): {calories:.0f} kcal"),
        html.P(f"Average AQI: {aqi:.0f} ({'Good' if aqi <= 50 else 'Moderate' if aqi <= 100 else 'Unhealthy'})"),
        html.P(f"Average Noise Level: {avg_noise:.1f} dB"),

    ]
            
    if noise_warning:
        text_items.append(html.P(noise_warning, style={"color": "orange"}))

    return html.Div(
        style={"display": "flex", "alignItems": "center", "gap": "40px"},
        children=[
            html.Div(text_items, style={"flex": "1"}),
            html.Div(
                [
                    dcc.Graph(figure=fig_bmi, config={"displayModeBar": False}),
                    dcc.Graph(figure=fig_aqi, config={"displayModeBar": False}),
                ],
                style={"width": "380px"},
            ),
        ],
    )


# ---------------------------------------------------------------------------
# 8. POKRETANJE APP --------------------------------------------------------
if __name__ == "__main__":
    app.run(debug=True, port=8050)