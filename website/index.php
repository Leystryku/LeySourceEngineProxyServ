<html>
	<head>
		<title>Nothing to see here...</title>
		<link rel="stylesheet" type="text/css" href="index.css">
		<link href="https://fonts.googleapis.com/css?family=Press+Start+2P" rel="stylesheet">

	</head>
	<body>
		<audio autoplay loop volume="0.5" id="musick" class="audio-example">
			<source src="index.mp3" type="audio/mpeg">
			<source src="index.ogg" type="audio/ogg">
			You will see this text if native audio playback is not supported.
		</audio>
		<script>
				function lowervolume()
				{
					var player = document.getElementById("musick");
					player.volume = 0.3;
				}
				window.onload = lowervolume;
		</script>

		<div id="text">
			<p><h1>Zyre</h1>
			<h2>The server revival project.</h2>

			<h3>The only thing leading you astray from victory is your loss</h3>

			made by <a href="https://steamcommunity.com/id/Leystryku" target="_blank">leystryku</a></p>
		</div>
</html>