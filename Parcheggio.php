<!DOCTYPE html>
<html lang="it">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Home Parcheggio</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f0f0f0;
        }

        .container {
            width: 80%;
            margin: 0 auto;
            padding: 20px;
            background-color: #fff;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }

        h1 {
            text-align: center;
            color: #333;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }

        th,
        td {
            padding: 12px;
            border: 1px solid #ddd;
            text-align: left;
        }

        th {
            background-color: #f2f2f2;
        }

        .green-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background-color: green;
            display: inline-block;
        }

        .red-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background-color: red;
            display: inline-block;
        }
    </style>

    <?php
    $connect = mysqli_connect("localhost", "root", "", "parcheggio");
    if (!$connect) {
        die("Connection failed: " . mysqli_connect_error());
    }

    if ($_SERVER['REQUEST_METHOD'] === 'GET') {
        if (isset($_GET['targa'])) {
            $targa = $_GET['targa'];
            $sql = "INSERT INTO posti (targa, orarioIng, stato) VALUES ('$targa', NOW(), TRUE)";

            if ($connect->query($sql) === TRUE)
                echo "Dati registrati con successo";
            else
                echo "Errore nell'inserimento dei nel database: " . $connect->error;
        }
        if (isset($_GET['update'])) {
            $update = $_GET['update'];
            $sql = "UPDATE posti SET stato = FALSE, orarioUscita = NOW() WHERE targa = '$update'";
            if ($connect->query($sql) === TRUE)
                echo "Dati registrati con successo";
            else
                echo "Errore nell'inserimento dei nel database: " . $connect->error;
        }
    }
    ?>


</head>

<body>
    <div class="container">
        <h1>Home Parcheggio</h1>

        <?php

        $query = "SELECT * FROM posti";
        $results = mysqli_query($connect, $query);
        if (!$results) {
            die("Query failed: " . mysqli_error($connect));
        }

        echo ('<table border="1"> <tr> <td> <b>Targa</b> </td> <td> <b>Orario Ingresso</b> </td> <td> <b>Orario Uscita</b> </td> <td> <b>Stato</b> </td> </tr> ');

        while ($row = mysqli_fetch_array($results)) {
            // Determina quale classe utilizzare in base allo stato
            $dotClass = ($row['stato']) ? 'green-dot' : 'red-dot';
            // Qui puoi elaborare i risultati, ad esempio:
            echo ("<tr> <td> " . $row['targa'] . "</td> <td> " . $row['orarioIng'] . "</td> <td> " . $row['orarioUscita'] . "</td> <td> <span class='$dotClass'></span> </td> </tr>");
        }
        echo ('</table>');
        // Chiudi la connessione al database
        mysqli_close($connect);
        ?>
    </div>


</body>

</html>